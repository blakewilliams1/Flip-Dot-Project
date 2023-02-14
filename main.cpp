#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>


#include <chrono>
#include <thread>

#include "FlipDotDriver.h"

using namespace std;

enum CONTROLLER_INPUT {
	NONE = 0,
	LEFT_D_PAD = 1,
	UP_D_PAD = 2,
	RIGHT_D_PAD = 3,
	DOWN_D_PAD = 4,
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

int highlightedMenuItem = 0;
SYSTEM_STATE currentSystemState = MENU;
FlipDotDriver displayDriver(56, 28); // 4 panels.

void refreshMenu() {
	displayDriver.clearDisplay();
	for (int i = 0; i < menuLabelsLength; i++) {
		displayDriver.drawText(menuLabels[i], 4, 22 - (i * 6));
	}
	displayDriver.drawText(">", 0, 22 - (highlightedMenuItem * 4));

	displayDriver.refreshEntireDisplay();
}

int main(int argc, char *argv[]) {
	FILE* depthCamFile;
	cout << "Starting controller listener" << endl;
	displayDriver.clearDisplay();
	displayDriver.drawText("loading", 1, 7, true);

	// Local developement pulls relative to .cpp files finshed version pulls relative to the binaries.
	FILE* controllerInput = popen("./ps1_driver", "r");
	if (!controllerInput) {
		cout << "Error starting controller listener!" << endl;
		return 1;
	}

	refreshMenu();
	// Indefinitely process input from controller and act upon it.
	char inputBuffer[3] = {};
	CONTROLLER_INPUT controllerValue = NONE;
	bool wasPressed = false;
	while(1) {
		// TODO: Figure out how to prevent a hang on camera locking up input handling.
		// Try to read a frame off the camera's file descriptor.
		if (currentSystemState == DEPTH_CAM) {

		}

		try {
			size_t bytesread =
			  fread(&inputBuffer, sizeof(char), sizeof(inputBuffer), controllerInput);
			if (bytesread == 0) {
				continue; // Maybe sleep a little to save power?
			} else {
				controllerValue =
				  static_cast<CONTROLLER_INPUT>(((int)inputBuffer[0] - 48) * 10 + ((int)inputBuffer[1] - 48));
				wasPressed = inputBuffer[2] == 1;
				cout << "Heard input: " << controllerValue << " is pressed: " << inputBuffer[2] << endl;
			}
		} catch (...) {
			cout << "Error was caught reading controller input!" << endl;
			pclose(controllerInput);
			return 1;
		}
		// Now handle the newly processed input.

		if (currentSystemState == MENU) {
			if (controllerValue == DOWN_D_PAD && wasPressed) {
				highlightedMenuItem = (highlightedMenuItem + 1) % menuLabelsLength;
				refreshMenu();
				continue;
			}
			if (controllerValue == UP_D_PAD && wasPressed) {
				highlightedMenuItem = (menuLabelsLength + highlightedMenuItem - 1) % menuLabelsLength;
				refreshMenu();
				continue;
			}
			if (controllerValue == X && wasPressed) {
				// Enter the depth cam mode.
				/*depthCamFile = popen("./rs-depth", "r");
				if (!depthCamFile) {
				  cout << "Error starting depth camera!" << endl;
				  continue;
				}*/

				cout << "starting depth camera!" << endl;
				currentSystemState = DEPTH_CAM;
			}
		} else if (currentSystemState == DEPTH_CAM) {
			if (controllerValue == O && wasPressed) {
				// Return to menus from the depth cam view.
				currentSystemState = MENU;
				refreshMenu();
				if (depthCamFile) {
					pclose(depthCamFile);
				}
			}
		}

		// Do something with the input...
	}

	return 0;
}
