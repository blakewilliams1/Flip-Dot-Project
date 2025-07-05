#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <vector>

#include "FlipDotDriver.h"
#include "ControllerInput.h"

enum SNAKE_DIRECTION {
  UP = 1,
  DOWN = 2,
  LEFT = 3,
  RIGHT = 4,
};

class SnakeSegment {
  public:
    int x;
    int y;

    SnakeSegment(int _x, int _y) {
      x = _x;
      y = _y;
    }

    bool operator==(SnakeSegment other) {
      return x == other.x && y == other.y;
    }
};

class SnakeGame {
  public:
    SnakeGame(FlipDotDriver& driver);
    bool maybeTickGame(CONTROLLER_INPUT controllerValue, bool wasPressed);
    void resetGame();

  private:
    FlipDotDriver& display;
    const int screenWidth = 56;
    const int screenHeight = 28;
    std::vector<SnakeSegment> segments;
    int score = 0;
    int lastGameTickTimestamp = 0;
    int startShowingGameOverScreenTimestamp = 0;
    int maxGameLoopTickHz = 15;
    SNAKE_DIRECTION direction = RIGHT;
    SnakeSegment pellet;

    bool tickGame();
    void movePellet();
    void renderScreen();
    bool isGameOver();
};

#endif // SNAKEGAME_H
