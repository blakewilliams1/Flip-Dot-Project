#include "FlipDotDriver.h"
//#include <pigpio.h> //"sudo pigpiod" starts the daemon, might need to be ran on startup

// C library headers
#include <stdio.h>
#include <string.h>
#include <cstdint>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <chrono>
#include <thread>


#include <cstdio>
#include <iostream>
using namespace std;
FlipDotDriver::FlipDotDriver(unsigned int width, unsigned int height) {
	displayWidth = width;
	displayHeight = height;
	numPanels = (width * height) / pixelsPerPanel;
	currentDisplayFrame = new byte[displayHeight * numPanels];

	// Open the UART in write-only mode on GPIO 14/15 (physical pins 8/10).
	uartPortAddress = open("/dev/ttyS0", O_WRONLY);
	if (uartPortAddress == -1) {
		cout << "Error openning '/dev/ttyS0'" << endl;
	}

	// Set the output buad rate to 57600.
	tcgetattr(uartPortAddress, &tty);
	cfsetospeed(&tty, B57600);
	tcsetattr(uartPortAddress, TCSADRAIN, &tty);
}

FlipDotDriver::~FlipDotDriver() {
	if (uartPortAddress != 0) {
		close(uartPortAddress);
	}
}

// Assumes the provided array is at least enough bits as the screen is pixels.
void FlipDotDriver::setRawDisplayData(const byte* rawData) {
	unsigned int rawDataBitIndex = 0;
	for (unsigned int currRow = 0; currRow < displayHeight; currRow++) {
		for (unsigned int j = 0; j < numPanels; j++) {
			// Iterate rows from left to right, top to bottom.
			byte rowValue = 0;
			for (unsigned int currBitIndex = 0; currBitIndex < 7; currBitIndex++) {
				byte currRawByte = rawData[rawDataBitIndex / 8];
				// If the current bit is high, add it into the proper spot on the current panel's row.
				if (currRawByte & (1 << (rawDataBitIndex % 8))) {
					rowValue += 1 << currBitIndex;
				}
				rawDataBitIndex++;
			}

			// Commit the built 7-byte-format row value into the display data buffer.
			currentDisplayFrame[(numPanels * currRow) + j] = rowValue;
		}
	}
}
uint8_t packetHeader[3] = {headerSignature, instantDisplayCommand, /*panelAddress=*/0};
uint8_t packetTail[1] = {payloadEndSignature};

void FlipDotDriver::refreshSinglePanel(unsigned int panelAddress) {
	byte currentPanelPayload[28] = {};
	for (int columnIndex = 0; columnIndex < 28; columnIndex++) {
		currentPanelPayload[columnIndex] = currentDisplayFrame[(columnIndex * numPanels) + panelAddress];
	}

	// Push the panel's frame to the data bus.
	packetHeader[2] = panelAddress;
	write(uartPortAddress, packetHeader, sizeof(packetHeader));
	// Pannel offset is also pannel address because addresses start at 0 and increase.
	write(uartPortAddress, currentPanelPayload, sizeof(currentPanelPayload));

	//Serial.write(payloadEndSignature);
	write(uartPortAddress, packetTail, sizeof(packetTail));

}

void FlipDotDriver::clearDisplay() {
	for (unsigned int i = 0; i < displayHeight * numPanels; i++) {
		currentDisplayFrame[i] = 0;
	}
}

void FlipDotDriver::refreshEntireDisplay() {
	// Assumes the length of the frame exactly corresponds to display size
	// Frame data is left to right rows, top to bottom
	// Panels left to right, and top to bottom within each panel
	// MSB on the left

	for (unsigned int pannelOffset = 0; pannelOffset < numPanels; pannelOffset++) {
		refreshSinglePanel(pannelOffset);
	}
}

// Mimics split flap display.
void FlipDotDriver::animateSplitFlapText(std::string originalText, unsigned int x, unsigned int y) {
	char intermediateText[originalText.length()] = {};
	// There is a total of 38 chars (0-9, a-z) mapped to flap positions. 50 is arbitrary as we always break out early.
	for (int flipIteration = 0; flipIteration < 50; flipIteration++) {
		bool stillIntermediate = false;
		// Generate the text to show based on the current flap iteration.
		for (unsigned int i = 0; i < originalText.length(); i++) {
			int realTextCharVal = getFlapIndex(originalText[i]);
			char intermediateChar = getCharFromFlapIndex(flipIteration);

			if (realTextCharVal <= flipIteration || realTextCharVal == -1) {
				intermediateText[i] = originalText[i];
			} else {
				stillIntermediate = true;
				intermediateText[i] = intermediateChar;
			}
		}

		// Draw the current intermediate flapped state.
		drawText(intermediateText, x, y);
		std::this_thread::sleep_for(std::chrono::milliseconds(25));

		// Break out of flap animation if we just flapped to the desired state.
		if (!stillIntermediate) {
			break;
		}
	}
}

void FlipDotDriver::drawText(std::string text, unsigned int x, unsigned int y) {
	drawText(text, x, y, false);
}

// Origin coordinate is anchored in lower left of screen and in pixel units.
void FlipDotDriver::drawText(std::string text, unsigned int x, unsigned int y, bool refreshPanel) {
	unsigned int backtrackOffsetX = 0;
	// For each character:
	for (unsigned int currCharIndex = 0; currCharIndex < text.length(); currCharIndex++) {
		// Multiples of 4 because chars are 3 dots wide and 1 space for padding.
		unsigned int currentCharPosX = x + (currCharIndex * 4) - backtrackOffsetX;
		byte* currentCharBitmap = getBitmapFromChar(text[currCharIndex]);

		// Draw out each character.
		for (unsigned int charRowIndex = 0; charRowIndex < textHeight; charRowIndex++) {
			byte currCharRowValue = currentCharBitmap[charRowIndex];
			// For each row of the current character:
			for (unsigned int charRowPixel = 0; charRowPixel < 3; charRowPixel++) {
				bool isPixelOn = currCharRowValue & (1 << charRowPixel);

				drawPixel(isPixelOn, currentCharPosX + charRowPixel, y + charRowIndex, false);
			}

			// Draw the spacer between each character.
			if (text.length() > 1) {
				drawPixel(false, currentCharPosX + 3, y + charRowIndex, false);
			}
		}

		// Pull the text std::string back on spaces and exclamations to cutt down excess padding.
		if (currentCharBitmap == spaceChar || currentCharBitmap == exclamationChar) {
			backtrackOffsetX += 2;
		}
	}

	if (!refreshPanel) {
		return;
	}
	// Refresh only the panels the text is being rendered across.
	unsigned int startingPanelAddress = x / 7;
	unsigned int endingPanelAddress = (x + (text.length() * 4) - backtrackOffsetX) / 7;
	for (unsigned int panelAddress = startingPanelAddress; panelAddress <= endingPanelAddress; panelAddress++) {
		refreshSinglePanel(panelAddress);
	}
}

void FlipDotDriver::drawPixel(bool isPixelOn, unsigned int x, unsigned int y) {
	drawPixel(isPixelOn, x, y, true);
}

void FlipDotDriver::drawPixel(bool isPixelOn, unsigned int x, unsigned int y, bool refreshPanel) {
	// Don't draw pixels that would be off the screen. It will wrap on display.
	if (x >= displayWidth || y >= displayHeight) {
		return;
	}

	// Seven pixels per panel, panel addresses are sequential from left to right.
	unsigned int panelAddress = x / 7;
	unsigned int rowIndex = (y * numPanels) + panelAddress;
	byte relevantRow = currentDisplayFrame[rowIndex];

	// Set the relevant bit to 0 or 1 depending on isPixelOn.
	unsigned int xCoordWithinPanel = x % 7;
	if (isPixelOn) {
		relevantRow = relevantRow | (1 << xCoordWithinPanel);
	} else {
		relevantRow = relevantRow & ~(1 << xCoordWithinPanel);
	}
	currentDisplayFrame[rowIndex] = relevantRow;

	if (refreshPanel) {
		refreshSinglePanel(panelAddress);
	}
}

char FlipDotDriver::getCharFromFlapIndex(unsigned int charVal) {
	// lowercase a through z.
	if (charVal <= 25) {
		return charVal + 97;
	}

	// 0-9 and semicolon
	if (charVal > 25 && charVal <= 35) {
		return charVal + 22;
	}

	// This means to default to origin text's char.
	return -1;
}

// A custom ordering is needed because the ascii values don't
// directly align with the ordering of split flap displays.
int FlipDotDriver::getFlapIndex(char c) {
	int charVal = (int)c;
	// lowercase a through z.
	if (charVal >= 97 && charVal <= 122) {
		return charVal - 97;
	}

	// 0-9 and semicolon
	if (charVal >= 48 && charVal <= 58) {
		return charVal - 22;
	}

	// fall through is no flap animation.
	return -1;
}

// Maps supported characters to their corresponding bitmaps.
byte* FlipDotDriver::getBitmapFromChar(char c) {
	switch (c) {
	case '0':
		return zeroChar;
	case '1':
		return oneChar;
	case '2':
		return twoChar;
	case '3':
		return threeChar;
	case '4':
		return fourChar;
	case '5':
		return fiveChar;
	case '6':
		return sixChar;
	case '7':
		return sevenChar;
	case '8':
		return eightChar;
	case '9':
		return nineChar;
	case 'a':
		return aChar;
	case 'b':
		return bChar;
	case 'c':
		return cChar;
	case 'd':
		return dChar;
	case 'e':
		return eChar;
	case 'f':
		return fChar;
	case 'g':
		return gChar;
	case 'h':
		return hChar;
	case 'i':
		return iChar;
	case 'j':
		return jChar;
	case 'k':
		return kChar;
	case 'l':
		return lChar;
	case 'm':
		return mChar;
	case 'n':
		return nChar;
	case 'o':
		return oChar;
	case 'p':
		return pChar;
	case 'q':
		return qChar;
	case 'r':
		return rChar;
	case 's':
		return sChar;
	case 't':
		return tChar;
	case 'u':
		return uChar;
	case 'v':
		return vChar;
	case 'w':
		return wChar;
	case 'x':
		return xChar;
	case 'y':
		return yChar;
	case 'z':
		return zChar;
	case ':':
		return colonChar;
	case ' ':
		return spaceChar;
	case '.':
		return periodChar;
	case '-':
		return dashChar;
	case '>':
		return greaterThanChar;
	case '!':
		return exclamationChar;
	// Return char is explict/intentional end of std::string.
	case '\n':
		return 0;
	default:
		return 0;
	}
}
