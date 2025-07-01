#include "FlippyBird.h"

#include <iostream>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

FlippyBird::FlippyBird(FlipDotDriver& _display) : display(_display) {}

void FlippyBird::resetGame() {
  score = 0;
  birdYPosition = height / 2;
  // Give some gap for the player to get used to the bird's position before encountering barriers.
  scrollOffsetX = 30;
  birdYVelocity = 0;
  barrierIndex = 0;
  startShowingGameOverScreenTimestamp = 0;
  upcomingBarrierHeights.clear();

  for (int i = 0; i < numBarriersQueued; i++) {
    upcomingBarrierHeights.emplace_back(rand() % (height - barrierGapSize - 1));
  }
}

bool FlippyBird::isGameOver() {
  return startShowingGameOverScreenTimestamp != 0;
}

bool FlippyBird::maybeTickGame(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  // The act of flapping immediately cancels all downward momentum and sets it to an upward motion.
  if (controllerValue == X && wasPressed) {
    /*if (birdYVelocity < 0) {
      birdYVelocity = 0.9;
    } else {
      birdYVelocity += 0.9;
    }*/
    birdYVelocity = 0;
    // Not bad, but would be cool to see it not skip the middle pixel.
    birdYPosition += 2;
  }
  if (birdYVelocity > 2) {
    birdYVelocity = 2;
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

bool FlippyBird::tickGame() {
  birdYPosition += birdYVelocity;
  birdYVelocity += birdAcceleration;
  scrollOffsetX -= 1;

  // Do the barrier collision calculation if game is not over.
  bool isPassingThroughFirstBarrier = scrollOffsetX == birdXPosition;
  bool isPassingThroughAnyBarrier = scrollOffsetX < 5 && ((scrollOffsetX - birdXPosition) % spaceBetweenBarriers) == 0;
  if ((isPassingThroughFirstBarrier || isPassingThroughAnyBarrier) && !isGameOver()) {
    // This is the screen coordinate of the bottom of the current barrier.
    int currBarrierYHeight = upcomingBarrierHeights.at(barrierIndex++);
    // Hit a barrier if true, so game over.
    if (birdYPosition <= currBarrierYHeight || birdYPosition >= currBarrierYHeight + barrierGapSize) {
      sleep(1);
      startShowingGameOverScreenTimestamp = clock();
    } else {
      // Flew through the barrier, increment score and update barriers.
      score++;
      // Only start updating the barriers after the first one.
      if (!isPassingThroughFirstBarrier) {
        upcomingBarrierHeights.emplace_back(rand() % (height - barrierGapSize - 1));
      }
    }
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

void FlippyBird::renderScreen() {
  display.clearDisplay();

  if (startShowingGameOverScreenTimestamp == 0) {
    // Draw the bird.
    display.drawPixel(true, 2, (int)birdYPosition, false);

    // Draw the barriers.
    for (unsigned int currBarrier = 0; currBarrier < upcomingBarrierHeights.size(); currBarrier++) {
      int currBarrierHeight = upcomingBarrierHeights.at(currBarrier);
      for (int i = 0; i < height; i++) {
        if (i < currBarrierHeight || i >= currBarrierHeight + barrierGapSize) {
          display.drawPixel(true, scrollOffsetX + (currBarrier * spaceBetweenBarriers), i, false);
        }
      }
    }

    // Draw the text after everything else to make sure it's on top.
    display.drawText(std::to_string(score), 5, 22, false);
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
