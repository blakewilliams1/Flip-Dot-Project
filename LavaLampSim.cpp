#include <iostream>
#include <unistd.h>
#include <limits>
#include <cmath>

#include "LavaLampSim.h"
#include "FlipDotDriver.h"
#include "ControllerInput.h"

using namespace std;

LavaLampSim::LavaLampSim(FlipDotDriver& _display) : display(_display) {
  for (int i = 0; i < numBalls; i++) {
    resetBallState(balls[i]);
  }
  for (int i = 0; i < numNegativeBalls; i++) {
    resetBallState(negativeBalls[i]);
  }
}

void LavaLampSim::resetBallState(Ball& ball) {
  ball.x = rand() % (maxX - 2 * margin) + margin;
  ball.y = rand() % (maxY - 2 * margin) + margin;
  float randomDirection = rand() % 360;
  // Root 2 will give the metaballs the same velocity as when they just moved along diagonals.
  ball.dx = std::sqrt(2) * std::cos(randomDirection);
  ball.dy = std::sqrt(2) * std::sin(randomDirection);
}

void LavaLampSim::resetDirections() {
  // Root 2 will give the metaballs the same velocity as when they just moved along diagonals.
  for (int i = 0; i < numBalls; i++) {
    float randomDirection = rand() % 360;
    balls[i].dx = std::sqrt(2) * std::cos(randomDirection);
    balls[i].dy = std::sqrt(2) * std::sin(randomDirection);
  }
  for (int i = 0; i < numNegativeBalls; i++) {
    float randomDirection = rand() % 360;
    negativeBalls[i].dx = std::sqrt(2) * std::cos(randomDirection);
    negativeBalls[i].dy = std::sqrt(2) * std::sin(randomDirection);
  }
}

void LavaLampSim::maybeTickSimulation(CONTROLLER_INPUT controllerValue, bool wasPressed) {
  if (controllerValue == TRIANGLE && wasPressed) {
    resetDirections();
  }

  // Check to see if enough time has passed to tick the game loop.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - lastSimulationTickTimestamp) / CLOCKS_PER_SEC;
  if (elapsedTimeSinceLastProcessedFrameSec >= ((double)1 / maxSimulationHz)) {
    tickSimulation();
    lastSimulationTickTimestamp = currClock;
  }
}

void LavaLampSim::simulateBall(Ball& ball) {
    if (ball.x + ball.dx >= maxX - margin ||
        ball.x + ball.dx < margin) {
      ball.dx *= -1;
    }
    if (ball.y + ball.dy >= maxY - margin ||
        ball.y + ball.dy < margin) {
      ball.dy *= -1;
    }

    ball.x += ball.dx;
    ball.y += ball.dy;
}

void LavaLampSim::tickSimulation() {
  display.clearDisplay();

  // Simulate the balls bouncing around the display.
  for (int i = 0; i < numBalls; i++) {
    simulateBall(balls[i]);
  }
  for (int i = 0; i < numNegativeBalls; i++) {
    simulateBall(negativeBalls[i]);
  }

  // Render each pixel.
  for (int x = 0; x < maxX; x++) {
    for (int y = 0; y < maxY; y++) {
      float sum = 0;

      // For each pixel, calculate how far all the metaballs are from it to determine if the pixel should be on.
      for (int k = 0; k < numBalls; k++) {
        sum += calculateDistSquared(balls[k], x, y);
      }
      for (int k = 0; k < numNegativeBalls; k++) {
        sum -= calculateDistSquared(balls[k], x, y);
      }

      // Turn the pixel on if there were enough metaballs sufficiently close enough to the given pixel.
      if (sum > pixelActivationThreshold) {
        display.drawPixel(true, x, y, false);
      }
    }
  }

  display.refreshEntireDisplay();
}

float LavaLampSim::calculateDistSquared(Ball& ball, int x, int y) {
  float dist_squared = (x - ball.x) * (x - ball.x) + (y - ball.y) * (y - ball.y);
  if (dist_squared == 0) {
    // If a metaball is exactly on top of the current pixel, it's definitely turning on or off. Return an arbitrarily
    // high value to be added (or subtracted if this is a negative metaball) and prevent division by zero.
    return 99999;
  } else {
    return (radius * radius) / dist_squared;
  }
}
