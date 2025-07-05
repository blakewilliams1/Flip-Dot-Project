#include "SnakeGame.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "FlipDotDriver.h"

SnakeGame::SnakeGame(FlipDotDriver& _display)
    : display(_display), pellet(screenWidth * 3 / 4, screenHeight / 2) {}

void SnakeGame::resetGame() {
  score = 0;
  startShowingGameOverScreenTimestamp = 0;
  lastGameTickTimestamp = 0;
  segments.clear();
  segments.emplace(segments.begin(), SnakeSegment(screenWidth / 2, screenHeight / 2));
}

void SnakeGame::movePellet() {
  // Keep looking for a new position for the pellet if it randomly gets put on the snake itself or the score.
  do {
    pellet.x = rand() % screenWidth;
    pellet.y = rand() % screenHeight;
  } while(std::find(segments.begin(), segments.end(), pellet) != segments.end()
      || (pellet.x < 5 && pellet.y >= screenHeight - 7));
}

bool SnakeGame::maybeTickGame(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  // Handle snake moving input Dont' allow it to reverse direction, it will collide with self.
  if (wasPressed) {
    switch (controllerValue) {
      case UP_D_PAD:
        if (direction != DOWN) direction = UP;
        break;
      case DOWN_D_PAD:
        if (direction != UP) direction = DOWN;
        break;
      case LEFT_D_PAD:
        if (direction != RIGHT) direction = LEFT;
        break;
      case RIGHT_D_PAD:
        if (direction != LEFT) direction = RIGHT;
        break;
      default: break;
    }
  }
  // Check to see if enough time has passed to tick the game loop.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - lastGameTickTimestamp) / CLOCKS_PER_SEC;
  if (elapsedTimeSinceLastProcessedFrameSec >= ((double)1 / maxGameLoopTickHz)) {
    lastGameTickTimestamp = currClock;
    return tickGame();
  }

  return true;
}

bool SnakeGame::tickGame() {
  SnakeSegment oldHead = segments.front();
  // Make a new head to move the snake in the direction it's currently going.
  SnakeSegment newHead(oldHead.x, oldHead.y);
  switch (direction) {
    case UP:
      newHead.y++;
      break;
    case DOWN:
      newHead.y--;
      break;
    case LEFT:
      newHead.x--;
      break;
    case RIGHT:
      newHead.x++;
    default: break;
  }

  // Check to see if we just ate the next pellet.
  bool justAtePellet = false;
  if (newHead == pellet) {
    justAtePellet = true;
    movePellet();
    score++;
  }

  // Check to make sure we don't run into the walls or self.
  bool hitSelf = std::find(segments.begin(), segments.end(), newHead) != segments.end();
  bool hitWall = newHead.x < 0 || newHead.x >= screenWidth || newHead.y < 0 || newHead.y >= screenHeight;
  if ((hitSelf || hitWall) && startShowingGameOverScreenTimestamp == 0) {
    sleep(1);
    startShowingGameOverScreenTimestamp = clock();
  } else if (startShowingGameOverScreenTimestamp == 0) {
    // If we hadn't hit the wall or self, move the snake forward by adding to the segments
    if (!justAtePellet) {
      segments.pop_back();
    }
    segments.emplace(segments.begin(), newHead);
  }

  renderScreen();

  // This the case when the game over screen is being shown. Quit out of the game after a few seconds.
  if (startShowingGameOverScreenTimestamp > 0) {
    clock_t currClock = clock();
    double elapsedTimeSinceLastProcessedFrameSec =
        (double)(currClock - startShowingGameOverScreenTimestamp) / CLOCKS_PER_SEC;

    // This is the signal to kill the game and return to the MENU state.
    if (elapsedTimeSinceLastProcessedFrameSec > 2.5) {
      return false;
    }
  }

  return true;
}

void SnakeGame::renderScreen() {
  display.clearDisplay();

  // Render the game or the game over screen, depending on the situation.
  if (startShowingGameOverScreenTimestamp == 0) {

    // Draw the snake
    for (SnakeSegment currSegment : segments) {
      display.drawPixel(true, currSegment.x, currSegment.y, false);
    }

    // Draw pellet and score.
    display.drawPixel(true, pellet.x, pellet.y, false);
    display.drawText(std::to_string(score), 2, screenHeight - 7);
  } else {
    // Show the game over screen.
    display.drawText("game over", 11, 14, false);
    int scoreXPosition = 23;
    if (score < 10) {
      scoreXPosition +=3;
    }
    display.drawText(std::to_string(score), scoreXPosition, 8, false);
  }

  display.refreshEntireDisplay();
}

