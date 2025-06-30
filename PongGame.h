#ifndef PONGGAME_H
#define PONGGAME_H

#include "FlipDotDriver.h"
#include "ControllerInput.h"

class PongGame
{
  public:
    PongGame(FlipDotDriver& driver);
    void startNewGame();
    // Returns true if the game is still being played.
    bool maybeTickGameLoop(CONTROLLER_INPUT controllerValue, bool wasPressed);

  private:
    FlipDotDriver& display;
    int leftScore;
    int rightScore;
    int leftPaddlePosition;
    int leftPaddleVelocity;
    int rightPaddlePosition;
    int rightPaddleVelocity;
    float ballX;
    float ballY;
    float ballXVelocity;
    float ballYVelocity;
    int minTimeIncrement;
    int lastGameTickTimestamp;
    int maxGameLoopTickHz = 15;
    int paddleLength = 7;
    int winningScore = 5;
    int width = 56;
    int height = 28;
    void tickGameLoop();
    void updateDisplay();

    // Separated just because I don't like them and would delete them if improvements to controller input state were made.
    bool leftPaddleDownPressed = false;
    bool leftPaddleUpPressed = false;
    bool rightPaddleDownPressed = false;
    bool rightPaddleUpPressed = false;
};

#endif // PONGGAME_H
