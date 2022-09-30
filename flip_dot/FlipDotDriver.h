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
static const unsigned int textHeight = 5;

class FlipDotDriver {
  public:
    FlipDotDriver(unsigned int width, unsigned int height);
    void setRawDisplayData(const byte* data);
    void drawText(String text, unsigned int x, unsigned int y);
    void drawText(String text, unsigned int x, unsigned int y, bool refreshPanel);
    void animateSplitFlapText(String text, unsigned int x, unsigned int y);
    void drawPixel(bool isPixelOn, unsigned int x, unsigned int y);
    void drawPixel(bool isPixelOn, unsigned int x, unsigned int y, bool refreshPanel);
    void clearDisplay();
    void refreshSinglePanel(unsigned int panelAddress);
    void refreshEntireDisplay();

  private:
    char getCharFromFlapIndex(unsigned int val);
    int getFlapIndex(char c);
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int numPanels;
    byte* currentDisplayFrame;


    byte oneChar[5] = {
      0b010,
      0b010,
      0b010,
      0b010,
      0b011
    };

    byte twoChar[5] = {
      0b111,
      0b001,
      0b111,
      0b100,
      0b111
    };

    byte threeChar[5] = {
      0b111,
      0b100,
      0b111,
      0b100,
      0b111
    };

    byte fourChar[5] = {
      0b100,
      0b100,
      0b111,
      0b101,
      0b101
    };

    byte fiveChar[5] = {
      0b111,
      0b100,
      0b111,
      0b001,
      0b111
    };

    byte sixChar[5] = {
      0b111,
      0b101,
      0b111,
      0b001,
      0b111
    };

    byte sevenChar[5] = {
      0b100,
      0b100,
      0b100,
      0b100,
      0b111
    };
    byte eightChar[5] = {
      0b111,
      0b101,
      0b111,
      0b101,
      0b111
    };

    byte nineChar[5] = {
      0b111,
      0b100,
      0b111,
      0b101,
      0b111
    };

    byte zeroChar[5] = {
      0b111,
      0b101,
      0b101,
      0b101,
      0b111
    };

    byte aChar[5] = {
      0b101,
      0b101,
      0b111,
      0b101,
      0b010
    };

    byte bChar[5] = {
      0b011,
      0b101,
      0b011,
      0b101,
      0b011
    };

    byte cChar[5] = {
      0b110,
      0b001,
      0b001,
      0b001,
      0b110
    };

    byte dChar[5] = {
      0b011,
      0b101,
      0b101,
      0b101,
      0b011
    };

    byte eChar[5] = {
      0b111,
      0b001,
      0b111,
      0b001,
      0b111
    };

    byte fChar[5] = {
      0b001,
      0b001,
      0b111,
      0b001,
      0b111
    };

    byte gChar[5] = {
      0b010,
      0b101,
      0b101,
      0b001,
      0b110
    };

    byte hChar[5] = {
      0b101,
      0b101,
      0b111,
      0b101,
      0b101
    };

    byte iChar[5] = {
      0b111,
      0b010,
      0b010,
      0b010,
      0b111
    };

    byte jChar[5] = {
      0b010,
      0b101,
      0b100,
      0b100,
      0b100
    };

    byte kChar[5] = {
      0b101,
      0b101,
      0b011,
      0b011,
      0b101
    };

    byte lChar[5] = {
      0b111,
      0b001,
      0b001,
      0b001,
      0b001
    };

    byte mChar[5] = {
      0b101,
      0b101,
      0b101,
      0b111,
      0b101
    };

    byte nChar[5] = {
      0b101,
      0b101,
      0b101,
      0b101,
      0b011
    };

    byte oChar[5] = {
      0b010,
      0b101,
      0b101,
      0b101,
      0b010
    };

    byte pChar[5] = {
      0b001,
      0b001,
      0b111,
      0b101,
      0b111
    };

    byte qChar[5] = {
      0b110,
      0b101,
      0b101,
      0b101,
      0b010
    };

    byte rChar[5] = {
      0b101,
      0b011,
      0b111,
      0b101,
      0b011
    };

    byte sChar[5] = {
      0b111,
      0b100,
      0b111,
      0b001,
      0b111
    };

    byte tChar[5] = {
      0b010,
      0b010,
      0b010,
      0b010,
      0b111
    };

    byte uChar[5] = {
      0b111,
      0b101,
      0b101,
      0b101,
      0b101
    };

    byte vChar[5] = {
      0b010,
      0b101,
      0b101,
      0b101,
      0b101
    };

    byte wChar[5] = {
      0b101,
      0b111,
      0b101,
      0b101,
      0b101
    };

    byte xChar[5] = {
      0b101,
      0b101,
      0b010,
      0b101,
      0b101
    };

    byte yChar[5] = {
      0b010,
      0b010,
      0b010,
      0b101,
      0b101
    };

    byte zChar[5] = {
      0b111,
      0b001,
      0b010,
      0b100,
      0b111
    };

    byte colonChar[5] = {
      0b000,
      0b010,
      0b000,
      0b010,
      0b000
    };

    byte spaceChar[5] = {
      0b000,
      0b000,
      0b000,
      0b000,
      0b000
    };

    byte periodChar[5] = {
      0b010,
      0b000,
      0b000,
      0b000,
      0b000
    };

    byte dashChar[5] = {
      0b000,
      0b000,
      0b111,
      0b000,
      0b000,
    };

    byte exclamationChar[5] = {
      0b010,
      0b000,
      0b010,
      0b010,
      0b010,
    };

    byte* getBitmapFromChar(char c);
};

#endif //FLIP_DOT_DRIVER_H
