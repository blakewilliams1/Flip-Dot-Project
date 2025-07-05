#include <cstdio>
#include <iostream>
#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>

// TODO: Delete this include after debugging
#include <time.h>

#include "ControllerInput.h"
#include "DepthCamera.h"
#include "FlipDotDriver.h"
#include "PongGame.h"
#include "LavaLampSim.h"
#include "FlippyBird.h"
#include "SnakeGame.h"

using namespace std;

enum SYSTEM_STATE {
	MENU = 0,
	DEPTH_CAM = 1,
	PONG = 2,
	LAVA_SIM = 3,
	FLIPPY_BIRD = 4,
	SNAKE = 5,
	WEATHER = 6,
};

const int menuLabelsLength = 6;
string menuLabels[menuLabelsLength] = {
	"depth cam",
	"pong",
	"lava sim",
	"flippy bird",
	"snake",
	"weather"
};

FILE* controllerInputFile = NULL;
char controllerInputBuffer[3] = {};
int highlightedMenuItem = 0;
CONTROLLER_INPUT controllerValue = NONE;
bool wasPressed = false;
SYSTEM_STATE currentSystemState = MENU;
FlipDotDriver displayDriver(56, 28); // 8 panels wide.
DepthCamera depthCameraInstance(displayDriver);
PongGame pongInstance(displayDriver);
LavaLampSim lavaInstance(displayDriver);
FlippyBird flippyBirdInstance(displayDriver);
SnakeGame snakeInstance(displayDriver);

void refreshMenu() {
	displayDriver.clearDisplay();
	// TODO: Bug here where some text is drawn one pixel too high. worse if drawing starts at x=6.
	// Panels really don't like 0b0001010 anywhere in the payload.
	// Able to draw horizontal line of pixels without issue, only issue when drawing text near top of the screen.

	int yOffset = 0;
	if (highlightedMenuItem > 3) {
    yOffset = (highlightedMenuItem - 3) * 6;
	}

	if (yOffset > 0) {
    displayDriver.drawText("^", 52, 22);
	}
	if (highlightedMenuItem != menuLabelsLength - 1) {
    displayDriver.drawText("~", 52, 1);
	}

	for (int i = 0; i < menuLabelsLength; i++) {
		displayDriver.drawText(menuLabels[i], 7, 21 - (i * 6) + yOffset);
	}

/*	displayDriver.drawPixel(true, 7-7, 26, false);
	displayDriver.drawPixel(true, 8-7, 25, false);
	displayDriver.drawPixel(true, 8-7, 24, false);
	displayDriver.drawPixel(true, 8-7, 23, false);
	displayDriver.drawPixel(true, 7-7, 22, false);*/
//  displayDriver.drawText("d", 6, 22);
 // displayDriver.drawText("e", 10-7, 22);
	displayDriver.drawText(">", 2, 21 - (highlightedMenuItem * 6) + yOffset);
	displayDriver.refreshEntireDisplay();
}

const char* weatherApiRequest =
  "curl -s wttr.in/?format=\"%l%0A%t%0A%C%0A%w%0A%u%0A%h\"";
void showWeatherInfo() {
  // Buffer to store each line of command output
  char buffer[128];

  // Open the command for reading its output
  FILE *fp = popen(weatherApiRequest, "r");
  if (fp == NULL) {
    printf("failed to get weather data");
  }

  // Extract response output.
  int lineIndex = 0;
  string locationName, temperature, weatherStatus, windSpeed, uvIndex, humidityPercent;
  while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    string line = buffer;
    // Replac the newline char with string terminating char. We needed the newline chars in the
    // custom request format but not anymore.
    if (!line.empty() && line.back() == '\n') {
      line.pop_back();
    }
    switch(lineIndex) {
      case 0: {
        int firstCommaIndex = line.find(',');
        if (firstCommaIndex > 0) {
          locationName = line.substr(0, firstCommaIndex);
        } else {
          locationName = line;
        }
        break;
      }
      case 1: {
        temperature = line;
        int degreeSymbolIndex = temperature.find("°");
        if (degreeSymbolIndex > 0) {
          temperature.erase(degreeSymbolIndex, 2);
        }
        break;
      }
      case 2:
        weatherStatus = line;
        break;
      case 3:
        // Erases the arrow symbol.
        line.erase(0,4);
        windSpeed = line;
        break;
      case 4:
        uvIndex = line;
        break;
      case 5:
        humidityPercent = line;
        break;
      default: break;
    }
    lineIndex++;
  }

  // Close the pipe. If it didn't close with success, show it.
  int returnCode = pclose(fp);

  // Show the weather if it was a success, or a warning on failure.
  displayDriver.clearDisplay();
  if (returnCode == 0) {
    displayDriver.drawText(locationName, 2, 21);
    displayDriver.drawText(temperature, 2, 15);
    displayDriver.drawText(humidityPercent, 26, 15);
    displayDriver.drawText("wind:" + windSpeed, 2, 9);
    //displayDriver.drawText(weatherStatus, 2, 3);
    //displayDriver.drawText("temp:" + temperature, 2, 15);
    //displayDriver.drawText("humid:" + humidityPercent, 2, 9);
    //displayDriver.drawText("uv:" + uvIndex, 39, 9);
  } else {
    displayDriver.drawText("error getting", 3, 15);
    displayDriver.drawText("weather", 15, 9);
  }
  displayDriver.refreshEntireDisplay();
}

FILE* initControllerProcess() {
	struct stat buffer;
	bool isControllerConnected = stat("/dev/input/js0", &buffer) == 0;
	// If there's no controller connected, it's basically impossible to provide input. Just boot up the depth cam.
  if (!isControllerConnected) {
    currentSystemState = DEPTH_CAM;
    depthCameraInstance.startDepthCamera();
  }

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
	// Check to see if some menu option as clicked.
	if (controllerValue != X || !wasPressed) {
    return;
  }
  switch (highlightedMenuItem) {
    case 0:
      // Enter the depth cam mode.
      depthCameraInstance.startDepthCamera();
      currentSystemState = DEPTH_CAM;
      break;
    case 1:
      // Enter pong mode.
      pongInstance.startNewGame();
      currentSystemState = PONG;
      break;
    case 2:
      currentSystemState = LAVA_SIM;
      break;
    case 3:
      flippyBirdInstance.resetGame();
      currentSystemState = FLIPPY_BIRD;
      break;
    case 4:
      currentSystemState = SNAKE;
      snakeInstance.resetGame();
      break;
    case 5:
      currentSystemState = WEATHER;
      displayDriver.clearDisplay();
      displayDriver.drawText("loading", 15, 12);
      displayDriver.refreshEntireDisplay();
      showWeatherInfo();
    default: break;

  }
}

void handleDepthCamState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
	if (controllerValue == O && wasPressed) {
		// Return to menus from the depth cam view and shut down the depth cam child process we have a reference to it
		// (implies it's live still).
		depthCameraInstance.stopDepthCamera();
		currentSystemState = MENU;
		refreshMenu();
		return;
	}

	depthCameraInstance.renderNextDepthFrame();
}

void handlePongState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  bool isGameStillGoing = pongInstance.maybeTickGameLoop(controllerValue, wasPressed);
	bool quitButtonPressed = controllerValue == O && wasPressed;

	if (quitButtonPressed || !isGameStillGoing) {
		// Return to menus from the pong game.
		currentSystemState = MENU;
		refreshMenu();
	}
}

void handleLavaSimState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  if (controllerValue == O && wasPressed) {
    currentSystemState = MENU;
		refreshMenu();
  }

  lavaInstance.maybeTickSimulation(controllerValue, wasPressed);
}

void handleFlippyBirdState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  bool isGameStillGoing = flippyBirdInstance.maybeTickGame(controllerValue, wasPressed);
  bool quitButtonPressed = controllerValue == O && wasPressed;
  if (quitButtonPressed || !isGameStillGoing) {
    currentSystemState = MENU;
		refreshMenu();
  }
}

void handleSnakeState(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  bool isGameStillGoing = snakeInstance.maybeTickGame(controllerValue, wasPressed);
  bool quitButtonPressed = controllerValue == O && wasPressed;
  if (quitButtonPressed || !isGameStillGoing) {
    currentSystemState = MENU;
		refreshMenu();
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
		//	cout << "Heard input: " << controllerValue << " is pressed: " << controllerInputBuffer[2] << endl;
		} catch (...) {
			cout << "Error was caught reading controller input!" << endl;
			pclose(controllerInputFile);
			return 1;
		}
		return 0;
}

int main(int argc, char *argv[]) {
	//cout << "Starting controller listener" << endl;
	displayDriver.clearDisplay();
	// A conditional preprocessor messsage on the panels to let user know if ready, if the screen resumes in the same state
	// it was turned off in. Macros are defined in Code::Blocks project build settings.
//	#ifdef RELEASE_BUILD
    displayDriver.drawText("ready", 18, 11);
    displayDriver.refreshEntireDisplay();
    sleep(2);
//	#endif
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
		} else if (currentSystemState == LAVA_SIM) {
      handleLavaSimState(controllerValue, wasPressed);
		} else if (currentSystemState == FLIPPY_BIRD) {
      handleFlippyBirdState(controllerValue, wasPressed);
		} else if (currentSystemState == SNAKE) {
      handleSnakeState(controllerValue, wasPressed);
		} else if (currentSystemState == WEATHER) {
      if (controllerValue == O && wasPressed) {
        currentSystemState = MENU;
        refreshMenu();
      }
		}
	}

	return 0;
}
