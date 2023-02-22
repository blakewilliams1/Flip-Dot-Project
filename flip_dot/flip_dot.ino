#include "FlipDotDriver.h"


FlipDotDriver displayDriver(56, 28);
// TODO: Verify if the displays take in a byte per row or 7 bits.
byte altCheckers[] = {
  0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
  0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
  0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
  0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
  0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
  0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
  0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
  0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
};

byte black_stripes[] = {0x80, 0x85, 0x00,
                        0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
                        0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
                        0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
                        0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
                        0x8F
                       };
byte white_stripes[] = {0x80, 0x85, 0x00,
                        0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
                        0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
                        0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F,
                        0x00, 0x7F, 0x00, 0x7F, 0x00, 0x7F, 0x00,
                        0x8F
                       };
byte all_white[] = {0x80, 0x85, 0x00,
                        0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                        0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                        0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                        0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                        0x8F
                       };
byte all_hex[] = {0x80, 0x85, 0x00,
                  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                  0x8F
                 };

void setup() {
  Serial.begin(57600);
  delay(100);
//  displayDriver.clearDisplay();
 // displayDriver.refreshEntireDisplay();

        Serial.write(all_white, 32);
  delay(2000);
        Serial.write(white_stripes, 32);
 // slowRefreshDotChecker(13000);
//  byte customTest[8 * 28] = {};
//  for (unsigned int i = 0; i < 8*28;i++){
//    customTest[i] = 0b10101010;
//  }
//  displayDriver.setRawDisplayData(customTest, 3,0, 10,10);
//  displayDriver.refreshEntireDisplay();
//  playPong();
//  fastRefreshDemo(1000);

// displayDriver.clearDisplay();
 //displayDriver.drawText("fuck you!!!", 3, 5);
 // displayDriver.refreshEntireDisplay();


//    delay(2000);
//    displayDriver.drawText("hack", 14, 19);
//    displayDriver.drawText("the", 15, 12);
//    displayDriver.drawText("planet", 10,5);
//    delay(2000);
//    bool toggle = true;
//    while (true) {
//      for (int i=0;i<55;i++) {
//        bool state = (i%2==0);
//        if (toggle) state = !state;
//        displayDriver.drawPixel(state, i, 0, false);
//        displayDriver.drawPixel(!state, i, 27, false);
//      }
//      for (int i=0;i<28;i++){
//        bool state = (i%2==0);
//        if (toggle) state = !state;
//        displayDriver.drawPixel(state, 0, i, false);
//        displayDriver.drawPixel(!state, 55, i, false);
//      }
//      toggle = !toggle;
//      displayDriver.refreshEntireDisplay();
//      delay(400);
//    }


//    displayDriver.animateSplitFlapText("send nudes!!!", 0,0);
  //  for (int i = 0; i < 60;i++) {
  //    displayDriver.clearDisplay();
  //    // TODO: for strings > 1 char, I need to clear the spaces between chars.
  //    displayDriver.drawText("send nudes!!!", i, 3);
  //    delay(125);
  //  }

//  byte customTest[8 * 28] = {};
//  for (unsigned int i = 0; i < 8*28;i++){
//    customTest[i] = 0b10101010;//i % 128;
//  }
//  displayDriver.setRawDisplayData(customTest);
//  displayDriver.refreshEntireDisplay();
}

//void testMenu() {
//  displayDriver.drawText(">", 3, 22, false);
//  displayDriver.drawText("pong", 7, 22, false);
//  displayDriver.drawText("depth cam", 7, 15, false);
//  displayDriver.drawText("lava lamp", 7, 8, false);
//  displayDriver.drawText("demo reel", 7, 1, false);
//  displayDriver.refreshEntireDisplay();
//}

void slowRefreshDotChecker(int durationMs) {
  displayDriver.clearDisplay();
  unsigned long startTimeMs = millis();
  boolean toggle = true;
  while(millis() < startTimeMs + durationMs) {
    if (toggle) {
      for (int i = 0; i < 8; i++) {
        all_white[2] = i;
        Serial.write(all_white, 32);
      }
    } else {
      displayDriver.refreshEntireDisplay();
    }
    toggle = !toggle;
    delay(4000);
  }
}

void fastRefreshDemo(int durationMs) {
  unsigned long startTimeMs = millis();
  boolean toggle = false;
  while(millis() < startTimeMs + durationMs) {
    // Iterate over each panel.
    for (int i = 0; i < 8; i++) {
      if (toggle) {
        white_stripes[2] = i;
        Serial.write(white_stripes, 32);
      } else {
        black_stripes[2] = i;
        Serial.write(black_stripes, 32);
      }
    }
    delay(100);
    toggle = !toggle;
  }
}

//void alternatingCheckers(int durationMs) {
//  unsigned long endTime = millis() + durationMs;
//
//  boolean stage = true;
//  while(endTime > millis()) {
//    byte* checkersFrame = stage ? checkers : altCheckers;
//   // displayDriver.sendFullFrameToDisplay(checkersFrame);
//    stage = !stage;
//    delay(750);
//  }
//}

unsigned int pongPlayerOneScore = 0;
unsigned int pongPlayerTwoScore = 0;
int playerOnePosition = 10;
int playerTwoPosition = 10;
int pongBallXPos = 20;
int pongBallYPos = 10;
int pongBallXVelocity = 1;
int pongBallYVelocity = 0;

void playPong() {
  resetPongBall();
  displayDriver.drawText("pong", 21, 23);

  while(true) {
    if (Serial.available() > 0) {
      char inputCommand = Serial.read();
      switch (inputCommand) {
        case 'q':
          playerOnePosition++;
          if (playerOnePosition > 20) playerOnePosition = 20;
          displayDriver.drawPixel(false, 0, playerOnePosition - 3, false);
          break;
        case 'a':
          playerOnePosition--;
          if (playerOnePosition < 2) playerOnePosition = 2;
          displayDriver.drawPixel(false, 0, playerOnePosition + 3, false);
          break;
        case 'w':
          playerTwoPosition++;
          if (playerTwoPosition > 20) playerTwoPosition = 20;
          displayDriver.drawPixel(false, 55, playerTwoPosition - 3, false);
          break;
        case 's':
          playerTwoPosition--;
          if (playerTwoPosition < 2) playerTwoPosition = 2;
          displayDriver.drawPixel(false, 55, playerTwoPosition + 3, false);
          break;
        case 'x':
          return;
      }
    }

    displayDriver.drawPixel(false, pongBallXPos, pongBallYPos, true);
    pongBallXPos += pongBallXVelocity;
    pongBallYPos += pongBallYVelocity;
    // Check pong ball bouncing on ceiling.
    if (pongBallYPos > 22) {
      pongBallYVelocity = -pongBallYVelocity;
      pongBallYPos = 22;
    }
    // Check pong ball bouncing on floor.
    if (pongBallYPos < 0) {
      pongBallYPos = -pongBallYPos;
      pongBallYVelocity = -pongBallYVelocity;
    }
    bool ballInPlayOneYRange =
        pongBallYPos < playerOnePosition + 3 && pongBallYPos > playerOnePosition - 3;
    bool ballInPlayTwoYRange =
        pongBallYPos < playerTwoPosition + 3 && pongBallYPos > playerTwoPosition - 3;
    if (pongBallXPos == 54 && ballInPlayTwoYRange) {
      // Bounced off player 2 paddle.
      pongBallXVelocity = -1;
      pongBallYVelocity = pongBallYPos - playerTwoPosition;
    } else if (pongBallXPos == 1 && ballInPlayOneYRange) {
      // Bounced off player 1 paddle.
      pongBallXVelocity = 1;
      pongBallYVelocity = pongBallYPos - playerOnePosition;
    } else if (pongBallXPos <= 0 && !ballInPlayOneYRange) {
      // Player one missed ball.
      pongPlayerTwoScore++;
      resetPongBall();
      delay(1500);
    } else if (pongBallXPos >= 55 && !ballInPlayTwoYRange) {
      // Player two missed ball.
      pongPlayerOneScore++;
      resetPongBall();
      delay(1500);
    }

    // End game conditions
    if (pongPlayerTwoScore >= 5 || pongPlayerOneScore >= 5) {
      endPongGame();
      break;
    }
    drawPongScreen();
    delay(50);
  }
}

void drawPongScreen() {
  //displayDriver.clearDisplay();
//  displayDriver.drawText(String(pongPlayerOneScore), 0, 23);
//  displayDriver.drawText(String(pongPlayerTwoScore), 53, 23);
  // Draw the paddles
  for(int i = -2; i < 3; i++) {
    displayDriver.drawPixel(true, 0, playerOnePosition + i, false);
    displayDriver.drawPixel(true, 55, playerTwoPosition + i, false);
  }
  displayDriver.drawPixel(true, pongBallXPos, pongBallYPos, true);
  displayDriver.refreshSinglePanel(0);
  displayDriver.refreshSinglePanel(5);
 // displayDriver.refreshEntireDisplay();
}

void resetPongBall() {
  displayDriver.drawText(String(pongPlayerOneScore), 0, 23);
  displayDriver.drawText(String(pongPlayerTwoScore), 53, 23);

  pongBallXPos = 20;
  pongBallYPos = 10;
  pongBallYVelocity = 0;
  pongBallXVelocity = pongPlayerOneScore > pongPlayerTwoScore ? -1 : 1;
  drawPongScreen();
}

void endPongGame() {
  displayDriver.clearDisplay();
  displayDriver.drawText("game over!", 3, 18);
  displayDriver.drawText(String(pongPlayerOneScore), 5, 10);
  displayDriver.drawText(String(pongPlayerTwoScore), 33, 10);
  delay(4000);
  displayDriver.clearDisplay();
  displayDriver.refreshEntireDisplay();

  // Reset all values.
  pongPlayerOneScore = 0;
  pongPlayerTwoScore = 0;
  playerOnePosition = 10;
  playerTwoPosition = 10;
  pongBallXPos = 20;
  pongBallYPos = 10;
  pongBallXVelocity = 1;
  pongBallYVelocity = 0;
  pongPlayerOneScore = 0;  
}

void loop() {


  //Serial.write(black_stripes, 32);
  //delay(1000);
  //Serial.write(white_stripes, 32);
  //delay(1000);


  //displayDriver.sendFullFrameToDisplay(black_stripes);
  //  Serial.println();
  // delay(10000);
}
