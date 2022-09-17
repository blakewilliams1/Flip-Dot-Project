#ifndef FLIP_DOT_DRIVER_H
#define FLIP_DOT_DRIVER_H

#include <stdint.h>
#include "Arduino.h"

#define byte uint8_t

static const unsigned int headerSignature = 0x80;
static const unsigned int instantDisplayCommand = 0x85;
static const unsigned int payloadEndSignature = 0x8F;
static const unsigned int pixelsPerPanel = 7 * 28;
// Height in pixels of each bitmap of the text char.
static const unsigned int textHeight = 7;

class FlipDotDriver {
  public:
    FlipDotDriver(unsigned int width, unsigned int height);
    void drawPixel(bool isPixelOn,  unsigned int x, unsigned int y);
    void clearDisplay();
    void refreshSinglePanel(unsigned int panelAddress);
    void refreshEntireDisplay();

  private:
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int numPanels;
    byte* currentDisplayFrame;

};

#endif //FLIP_DOT_DRIVER_H
