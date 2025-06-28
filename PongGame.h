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

    virtual ~PongGame();


  private:
    FlipDotDriver& display;
    int leftScore;
    int rightScore;
    int leftPaddlePosition;
    int leftPaddleVelocity;
    int rightPaddlePosition;
    int rightPaddleVelocity;
    int ballX;
    int ballY;
    int ballXVelocity;
    int ballYVelocity;
    int minTimeIncrement;
    int lastGameTickTimestamp;
    int maxGameLoopTickHz = 10;
    int paddleLength = 5;
    int winningScore = 1;
    int width = 56;
    int height = 28;
    bool updateBallPosition = false;
    void tickGameLoop();
    void updateDisplay();
};

#endif // PONGGAME_H
