#include "FlipDotDriver.h"

FlipDotDriver::FlipDotDriver(unsigned int width, unsigned int height) {
  displayWidth = width;
  displayHeight = height;
  numPanels = (width * height) / pixelsPerPanel;
  currentDisplayFrame = new byte[displayHeight * numPanels];
}

void FlipDotDriver::refreshSinglePanel(unsigned int panelAddress) {
  byte currentPanelPayload[28] = {};
    for (int columnIndex = 0; columnIndex < 28; columnIndex++) {
      currentPanelPayload[columnIndex] = currentDisplayFrame[(columnIndex * numPanels) + panelAddress];
    }

    // Push the panel's frame to the data bus.
    Serial.write(headerSignature);
    Serial.write(instantDisplayCommand);
    // Pannel offset is also pannel address because addresses start at 0 and increase.
    Serial.write(panelAddress);
    Serial.write(currentPanelPayload, pixelsPerPanel);
    Serial.write(payloadEndSignature);
}

void FlipDotDriver::clearDisplay() {
  for (unsigned int i = 0; i < displayHeight * numPanels; i++) {
    currentDisplayFrame[i] = 0;
  }

  refreshEntireDisplay();
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

// Origin coordinate is anchored in upper left of screen and in pixel units.
void FlipDotDriver::drawText(String text, unsigned int x, unsigned int y) {
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

    // Pull the text string back on spaces and exclamations to cutt down excess padding.
    if (currentCharBitmap == spaceChar || currentCharBitmap == exclamationChar) {
      backtrackOffsetX+=2;
    }
  }

  // TODO: This can probably by simplified to only refresh changed panels.
  refreshEntireDisplay();
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

byte* FlipDotDriver::getBitmapFromChar(char c) {
    switch(c) {
      case '0': return zeroChar;
      case '1': return oneChar;
      case '2': return twoChar;
      case '3': return threeChar;
      case '4': return fourChar;
      case '5': return fiveChar;
      case '6': return sixChar;
      case '7': return sevenChar;
      case '8': return eightChar;
      case '9': return nineChar;
      case 'a': return aChar;
      case 'b': return bChar;
      case 'c': return cChar;
      case 'd': return dChar;
      case 'e': return eChar;
      case 'f': return fChar;
      case 'g': return gChar;
      case 'h': return hChar;
      case 'i': return iChar;
      case 'j': return jChar;
      case 'k': return kChar;
      case 'l': return lChar;
      case 'm': return mChar;
      case 'n': return nChar;
      case 'o': return oChar;
      case 'p': return pChar;
      case 'q': return qChar;
      case 'r': return rChar;
      case 's': return sChar;
      case 't': return tChar;
      case 'u': return uChar;
      case 'v': return vChar;
      case 'w': return wChar;
      case 'x': return xChar;
      case 'y': return yChar;
      case 'z': return zChar;
      case ':': return colonChar;
      case ' ': return spaceChar;
      case '.': return periodChar;
      case '-': return dashChar;
      case '!': return exclamationChar;
      // Return char is explict/intentional end of string.
      case '\n': return 0;
      default: return 0;
    }
}
