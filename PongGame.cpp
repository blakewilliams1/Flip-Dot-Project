#include <iostream>
#include <unistd.h>

#include "PongGame.h"
#include "ControllerInput.h"

/*PongGame::PongGame(FlipDotDriver& _display) {
  display = _display;
}*/
PongGame::PongGame(FlipDotDriver& _display) : display(_display) {}

PongGame::~PongGame() {
  //dtor
}

void PongGame::startNewGame() {
  rightScore = 0;
  leftScore = 0;
  ballX = width / 2;
  ballY = height / 2;
  ballXVelocity = 1;
  ballYVelocity = 0;
  rightPaddlePosition = height / 2 - paddleLength / 2;
  leftPaddlePosition = height / 2 - paddleLength / 2;
}

bool PongGame::maybeTickGameLoop(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  // Check to see if a win condition has been reached.
  if (rightScore >= winningScore) {
    display.clearDisplay();
    display.drawText("right wins", 9, 11);
    display.refreshEntireDisplay();
    sleep(3);
    return false;
  }
  if (leftScore >= winningScore) {
    display.clearDisplay();
    display.drawText("left wins", 11, 11);
    display.refreshEntireDisplay();
    sleep(3);
    return false;
  }

  // Update hardware button state even if not ticking the game loop forward.
  //bool forceUpdatePaddles = false;
  switch(controllerValue) {
    case L1:
      if (wasPressed) {
        leftPaddleVelocity = 1;
        //forceUpdatePaddles = true;
      } else {
        leftPaddleVelocity = 0;
      }
      break;
    case L2:
      if (wasPressed) {
        leftPaddleVelocity = -1;
        //forceUpdatePaddles = true;
      } else {
        leftPaddleVelocity = 0;
      }
      break;
    case R1:
      if (wasPressed) {
        rightPaddleVelocity = 1;
        //forceUpdatePaddles = true;
      } else {
        rightPaddleVelocity = 0;
      }
      break;
    case R2:
      if (wasPressed) {
        rightPaddleVelocity = -1;
        //forceUpdatePaddles = true;
      } else {
        rightPaddleVelocity = 0;
      }
      break;
    default: break;
  }

  // Check to see if enough time has passed to tick the game loop.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - lastGameTickTimestamp) / CLOCKS_PER_SEC;
  if (elapsedTimeSinceLastProcessedFrameSec >= ((double)1 / maxGameLoopTickHz)) {
    tickGameLoop();
    lastGameTickTimestamp = currClock;
  } /*else if (forceUpdatePaddles) {
  //
    //updatePaddles();
  }*/

  return true;
}

void PongGame::tickGameLoop() {
  // Increment the position of the paddles.
  rightPaddlePosition += rightPaddleVelocity;
  if (rightPaddlePosition <= 0) {
    rightPaddlePosition = 0;
  }
  if (rightPaddlePosition >= height - paddleLength - 1) {
    rightPaddlePosition = height - paddleLength - 1;
  }
  leftPaddlePosition += leftPaddleVelocity;
  if (leftPaddlePosition <= 0) {
    leftPaddlePosition = 0;
  }
  if (leftPaddlePosition >= height - paddleLength - 1) {
    leftPaddlePosition = height - paddleLength - 1;
  }

  // Increment the position of the ball only every other tick to make the paddles move faster than the ball. This is
  // preferrable to just +=2 on paddle position because in that case there would just be positions not possible for
  // the paddle to reside in.
  if (updateBallPosition) {
    ballX += ballXVelocity;
    ballY += ballYVelocity;
  }
  updateBallPosition = !updateBallPosition;

  // Bounce ball off floor and ceiling.
  if (ballY <= 0) {
    ballY = 0;
    ballYVelocity = -ballYVelocity;
  }
  if (ballY >= height - paddleLength - 1) {
    ballY = height - paddleLength - 1;
    ballYVelocity = -ballYVelocity;
  }

  // Bounce ball off paddles.
  if (ballX == 1 && (ballY >= leftPaddlePosition && ballY <= leftPaddlePosition + paddleLength)) {
    ballXVelocity = -ballXVelocity;
    ballYVelocity = -ballYVelocity;
  }
  if (ballX == width - 2 && (ballY >= rightPaddlePosition && ballY <= rightPaddlePosition + paddleLength)) {

    ballXVelocity = -ballXVelocity;
    ballYVelocity = -ballYVelocity;
  }

  // Check for score. Only way for ballX to be these values is if it didn't bounce off the paddles.
  if (ballX <= 0) {
    rightScore++;
    ballX = width / 2;
    ballY = height / 2;
    ballXVelocity = 1;
    ballYVelocity = 0;
  }
  if (ballX >= width - 1) {
    leftScore++;
    ballX = width / 2;
    ballY = height / 2;
    ballXVelocity = -1;
    ballYVelocity = 0;
  }

  // Finally, update the display to refect current game state.
  updateDisplay();
}

void PongGame::updateDisplay() {
  display.clearDisplay();
  for (int i = 0; i < height; i++) {
    display.drawPixel(true, width / 2 - 1, i, false);
  }
  display.drawText(std::to_string(leftScore), width / 2 - 5, 23, false);
  display.drawText(std::to_string(rightScore), width / 2 + 1, 23, false);
  display.drawPixel(true, ballX, ballY);

  for (int i = 0; i < paddleLength; i ++) {
    display.drawPixel(true, 0, leftPaddlePosition + i, false);
    display.drawPixel(true, width - 1, rightPaddlePosition + i, false);
  }

  display.refreshEntireDisplay();
}
