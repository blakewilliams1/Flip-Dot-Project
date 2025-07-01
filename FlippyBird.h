#ifndef FLIPPYBIRD_H
#define FLIPPYBIRD_H

#include "FlipDotDriver.h"
#include "ControllerInput.h"

#include <deque>

class FlippyBird
{
  public:
    FlippyBird(FlipDotDriver& driver);
    bool maybeTickGame(CONTROLLER_INPUT controllerValue, bool wasPressed);
    void resetGame();

  private:
    FlipDotDriver& display;
    int score = 0;
    const int height = 28;
    int birdXPosition = 2;
    float birdYPosition = height / 2;
    float birdYVelocity = 0;
    const float birdAcceleration = -0.25;
    int scrollOffsetX = 0;
    int maxGameLoopTickHz = 12;
    int lastGameTickTimestamp = 0;
    int startShowingGameOverScreenTimestamp = 0;
    int numBarriersQueued = 6;
    int barrierGapSize = 7;
    std::deque<int> upcomingBarrierHeights;
    int spaceBetweenBarriers = 12;
    int barrierIndex = 0;

    bool tickGame();
    void renderScreen();
    bool isGameOver();
};

#endif // FLIPPYBIRD_H
