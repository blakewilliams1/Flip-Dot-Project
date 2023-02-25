#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <fcntl.h>

#include <chrono>
#include <thread>

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
char controllerInputBuffer[3] = {};
char cameraInputBuffer[1652] = {}; // 59 * 28 pixels
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

FILE* initDepthCamProcess() {
	// Local developement pulls relative to .cpp files, finshed version pulls relative to the binaries.
	FILE* processFilePointer = popen("./rs-depth", "r");
	if (!processFilePointer) {
		cout << "Error starting depth cam process!" << endl;
		return processFilePointer;
	}
	int controllerInputFd = fileno(processFilePointer);
	int controllerInputFdFlags = fcntl(controllerInputFd, F_GETFL, 0);
	fcntl(controllerInputFd, F_SETFL, controllerInputFdFlags | O_NONBLOCK);

	return processFilePointer;
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
		displayDriver.drawText("loading", 15, 12, true);
		depthCamFile = popen("./rs-depth", "r");
		if (!depthCamFile) {
			cout << "Error starting depth camera!" << endl;
			return;
		}
		// Set the file descriptor to be nonblocking reads.
		int depthCamFd = fileno(depthCamFile);
		int depthCamFdFlags = fcntl(depthCamFd, F_GETFL, 0);
		fcntl(depthCamFd, F_SETFL, depthCamFdFlags | O_NONBLOCK);

		cout << "starting depth camera!" << endl;
		currentSystemState = DEPTH_CAM;
	}
}


void handleDepthCamState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
	if (controllerValue == O && wasPressed) {
		// Return to menus from the depth cam view.
		currentSystemState = MENU;
		refreshMenu();
		if (depthCamFile) {
			pclose(depthCamFile);
		}
		return;
	}

	// Read depth camera data. All values > 2 are on pixels. Incoming buffer will be 59 x 28 pixels
	//cout << "about to read cam data" << endl;
	size_t cameraReadStatus =
	  fread(&cameraInputBuffer, sizeof(char), sizeof(cameraInputBuffer), depthCamFile);

	byte camDisplayBuffer[196] = {};
	if (cameraReadStatus == 0) {
		// Convert all values into a raw display buffer and send to display.
		int cameraInputBufferIndex = 0;
		for (int i = 0; i < 196; i++) {
			byte builtRow = 0;
			for (int j = 0; j < 8; j++) {
				// The pixel is on if the value of the depth is greater than 2.
				bool isPixelOn = ((int)cameraInputBuffer[cameraInputBufferIndex] - 48) > 2;
				builtRow |= (((int)isPixelOn)<<j);
				cameraInputBufferIndex++;
			}
			camDisplayBuffer[i] = builtRow;
		}
		//printf(camDisplayBuffer);
		displayDriver.setRawDisplayData(camDisplayBuffer);
		displayDriver.refreshEntireDisplay();
	}
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
