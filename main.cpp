#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <fcntl.h>

#include <chrono>
#include <thread>
#include <csignal>
#include <unistd.h>

// TODO: Delete this include after debugging
#include <time.h>

#include "FlipDotDriver.h"

using namespace std;

enum CONTROLLER_INPUT {
	NONE = 0,
	RIGHT_D_PAD = 1,
	DOWN_D_PAD = 2,
	LEFT_D_PAD = 3,
	UP_D_PAD = 4,
	TRIANGLE = 5,
	O = 6,
	X = 7,
	SQUARE = 8,
	L1 = 9,
	R1 = 10,
	L2 = 11,
	R2 = 12,
};

enum SYSTEM_STATE {
	MENU = 0,
	DEPTH_CAM = 1,
	PONG = 2,
	PIXEL_TESTING = 3,
};

string menuLabels[3] = {
	"depth cam",
	"pong",
	"demo",
};
int menuLabelsLength = sizeof(menuLabels) / sizeof(string);

FILE* controllerInputFile = NULL;
FILE* depthCamFile = NULL;
pid_t depthCamPid = 0;
char controllerInputBuffer[3] = {};
char cameraInputBuffer[1792] = {}; // 64 * 28 pixels
int highlightedMenuItem = 0;
CONTROLLER_INPUT controllerValue = NONE;
bool wasPressed = false;
SYSTEM_STATE currentSystemState = MENU;
FlipDotDriver displayDriver(56, 28); // 4 panels.

void refreshMenu() {
	displayDriver.clearDisplay();
	for (int i = 0; i < menuLabelsLength; i++) {
		displayDriver.drawText(menuLabels[i], 5, 22 - (i * 6));
	}
	displayDriver.drawText(">", 1, 22 - (highlightedMenuItem * 6));
	displayDriver.refreshEntireDisplay();
}

FILE* initControllerProcess() {
	// Local development pulls relative to .cpp files, finshed version pulls relative to the binaries.
	FILE* processFilePointer = popen("./ps1_driver", "r");
	if (!processFilePointer) {
		cout << "Error starting controller listener!" << endl;
		return processFilePointer;
	}
	int controllerInputFd = fileno(processFilePointer);
	int controllerInputFdFlags = fcntl(controllerInputFd, F_GETFL, 0);
	fcntl(controllerInputFd, F_SETFL, controllerInputFdFlags | O_NONBLOCK);

	return processFilePointer;
}

void initDepthCamProcess() {
  depthCamFile = popen("./rs-depth", "r");
  if (!depthCamFile) {
    cout << "Error starting depth camera!" << endl;
    return;
  }

  // Read the PID of the newly created child process before setting reads to non-blocking.
  // We need to know the PID to give it the kill signal when exiting from depth cam state.
	size_t pidReadStatus =
      fread(&depthCamPid, sizeof(pid_t), 1, depthCamFile);

  // Set the file descriptor to be nonblocking reads.
  int depthCamFd = fileno(depthCamFile);
  int depthCamFdFlags = fcntl(depthCamFd, F_GETFL, 0);
  fcntl(depthCamFd, F_SETFL, depthCamFdFlags | O_NONBLOCK);

  cout << "starting depth camera!" << endl;
  currentSystemState = DEPTH_CAM;
}

void handleMenuState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
	if (controllerValue == DOWN_D_PAD && wasPressed) {
		highlightedMenuItem = (highlightedMenuItem + 1) % menuLabelsLength;
		refreshMenu();
		return;
	}
	if (controllerValue == UP_D_PAD && wasPressed) {
		highlightedMenuItem = (menuLabelsLength + highlightedMenuItem - 1) % menuLabelsLength;
		refreshMenu();
		return;
	}
	if (controllerValue == X && wasPressed && highlightedMenuItem == 0) {
		// Enter the depth cam mode.
		displayDriver.clearDisplay();
  	displayDriver.drawText("loading", 15, 12);
  	displayDriver.refreshEntireDisplay();
		initDepthCamProcess();
	}
}

clock_t timeOfLastProcessedFrame = 0;
void handleDepthCamState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
	if (controllerValue == O && wasPressed) {
		// Return to menus from the depth cam view.
		currentSystemState = MENU;
		refreshMenu();
		// Shut down the depth cam child process we have a reference to it (implies it's live still).
		if (depthCamPid > 0) {
      kill(depthCamPid, SIGTERM);
      depthCamPid = -1;
      depthCamFile = nullptr;
		}
		return;
	}

	// Read depth camera data. All values > 2 are on pixels. Incoming buffer will be 64 x 28 pixels
	// but the first 5 values of each row should be trimmed off becaues they're dead for some reason.
	// The actual panel 56 x 28 in size.
	size_t cameraReadStatus =
	  fread(&cameraInputBuffer, sizeof(char), sizeof(cameraInputBuffer), depthCamFile);

	// Not bytes read this pass.
	if (cameraReadStatus == 0) {
    return;
	}

	// Bytes have been read, and we're assuming that's a full frame of data! We're recieving at 30fps but present at
	// whatever the fastest framerate I can get by with. Initial tests showed that to be about 10fps for full screen
	// refreshes, but in practice the depth cam data does change every single pixel at the same time and we can bump
	// it up a little more.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - timeOfLastProcessedFrame) / CLOCKS_PER_SEC;
  int maxRefreshFrameRateHz = 20;
  if (elapsedTimeSinceLastProcessedFrameSec < ((double)1 / maxRefreshFrameRateHz)) {
    return;
  }
  // The size of the display in pixels, divided by 8 because each element of this array holds 8 pixels of data.
  byte rawDisplayBuffer[(56 * 28) / 8] = {};
  timeOfLastProcessedFrame = currClock;
  // There are 28 rows on the display, process them one at a time.
  int cameraBufferRowWidth = 64;
  // The nested for loops are designed to iterate over the desired elements of the camera data, but that is not aligned
  // with the indexing of the output buffer for displays. Use this index variable to track where we should put the
  // processed data within the display buffer.
  int currDisplayPixelIndex = 0;
  for (int currRow = 0; currRow < 28; currRow++) {
    for (int currPixel = 8; currPixel < cameraBufferRowWidth; currPixel++) {
      // Get the data at the camera pixel.
      int currDepthValue = cameraInputBuffer[(currRow * cameraBufferRowWidth) + currPixel] - '0';
      bool isPixelOn = currDepthValue > 1;
      // Set the data into the display input buffer.
      byte relevanteByte = rawDisplayBuffer[currDisplayPixelIndex / 8];
      if (isPixelOn) {
        // Force bit to be high.
        relevanteByte |= 1 << (currDisplayPixelIndex % 8);
      }
      // Save the modification back into the display buffer and increment the index of which bit to set next.
      rawDisplayBuffer[currDisplayPixelIndex / 8] = relevanteByte;
      currDisplayPixelIndex++;
    }
  }
  //printf("%.1792s\n", cameraInputBuffer);
  //cout << "going to process: " << cameraReadStatus << " bytes. elapsedTimeSinceLastProcessedFrame: " << elapsedTimeSinceLastProcessedFrameSec << endl;
  displayDriver.clearDisplay();
  displayDriver.setRawDisplayData(rawDisplayBuffer);
  displayDriver.refreshEntireDisplay();
}

void handlePongState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
	if (controllerValue == O && wasPressed) {
		// Return to menus from the pong game.
		currentSystemState = MENU;
		refreshMenu();
		return;
	}
}


int checkForNewControllerInput() {
	try {
			size_t bytesread =
			  fread(&controllerInputBuffer, sizeof(char), sizeof(controllerInputBuffer), controllerInputFile);

			if (bytesread == 0) {
				controllerValue = NONE;
				wasPressed = false;
				return 0;
			}

			controllerValue =
			  static_cast<CONTROLLER_INPUT>((10 * (int)controllerInputBuffer[0] + (int)controllerInputBuffer[1]) - 528);
			wasPressed = controllerInputBuffer[2] == '1';
			cout << "Heard input: " << controllerValue << " is pressed: " << controllerInputBuffer[2] << endl;
		} catch (...) {
			cout << "Error was caught reading controller input!" << endl;
			pclose(controllerInputFile);
			return 1;
		}
		return 0;
}

int main(int argc, char *argv[]) {
	cout << "Starting controller listener" << endl;
	displayDriver.clearDisplay();
	refreshMenu();

	controllerInputFile = initControllerProcess();
	// TODO: Consider sleeping a little to save power sometimes?
	if (!controllerInputFile) {
		return 1;
	}

	// Indefinitely process input from controller and act upon it.
	while(1) {
		int controllerStatus = checkForNewControllerInput();
		if (controllerStatus != 0) {
			return controllerStatus;
		}

		// Now handle the newly processed input.
		if (currentSystemState == MENU) {
			handleMenuState(controllerValue, wasPressed);
		} else if (currentSystemState == DEPTH_CAM) {
			handleDepthCamState(controllerValue, wasPressed);
		} else if (currentSystemState == PONG) {
			handlePongState(controllerValue, wasPressed);
		}
	}

	return 0;
}
