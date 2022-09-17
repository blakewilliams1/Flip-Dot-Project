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
      // TODO: This needs to be revised. If the frame does not contain spacing then we need to account for that
      currentPanelPayload[columnIndex] = currentDisplayFrame[(columnIndex * numPanels) + panelAddress];
//      }
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
}

void FlipDotDriver::refreshEntireDisplay() {
  // Assumes the length of the frame exactly corresponds to display size
  // Frame data is left to right rows, top to bottom
  // Panels left to right, and top to bottom within each panel
  // MSB on the left

  for (unsigned int pannelOffset = 0; pannelOffset < numPanels; pannelOffset++) {
    refreshSinglePanel(pannelOffset);
  }

  // Update the content of class member 'currentDisplayFrame'.
  //memcpy(currentDisplayFrame, frame, displayHeight * numPanels);
}

void FlipDotDriver::drawPixel(bool isPixelOn, unsigned int x, unsigned int y) {
  // Seven pixels per panel, panel addresses are sequential from left to right.
  unsigned int panelAddress = x / 7;
  unsigned int xCoordWithinPanel = x % 7;
  unsigned int frameIndex = (numPanels - 1) + y;
  byte relevantRow = currentDisplayFrame[frameIndex];

  // Set the relevant bit to 0 or 1 depending on isPixelOn.
  if (isPixelOn) {
    relevantRow = relevantRow | (1 << xCoordWithinPanel);
  } else {
    relevantRow = relevantRow & ~(1 << xCoordWithinPanel);
  }
  currentDisplayFrame[frameIndex] = relevantRow;

  refreshSinglePanel(panelAddress);
}

