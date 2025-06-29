#include <iostream>
#include <unistd.h>
#include <limits>

#include "LavaLampSim.h"
#include "FlipDotDriver.h"
#include "ControllerInput.h"

using namespace std;

LavaLampSim::LavaLampSim(FlipDotDriver& _display) : display(_display) {
  for (int i = 0; i < numBalls; i++) {
    balls[i].x = rand() % (maxX - 2 * margin) + margin;
    balls[i].y = rand() % (maxY - 2 * margin) + margin;
    balls[i].dx = (rand() % 2 == 0) ? -1 : 1;
    balls[i].dy = (rand() % 2 == 0) ? -1 : 1;
  }
}

void LavaLampSim::maybeTickSimulation() {
  // Check to see if enough time has passed to tick the game loop.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - lastSimulationTickTimestamp) / CLOCKS_PER_SEC;
  if (elapsedTimeSinceLastProcessedFrameSec >= ((double)1 / maxSimulationHz)) {
    tickSimulation();
    lastSimulationTickTimestamp = currClock;
  }
}

void LavaLampSim::tickSimulation() {
  display.clearDisplay();

  // Simulate the balls bouncing around the display.
  for (int i = 0; i < numBalls; i++) {
    if (balls[i].x + balls[i].dx >= maxX - margin ||
        balls[i].x + balls[i].dx < margin)
      balls[i].dx *= -1;

    if (balls[i].y + balls[i].dy >= maxY - margin ||
        balls[i].y + balls[i].dy < margin) {
      balls[i].dy *= -1;
      }

    balls[i].x += balls[i].dx;
    balls[i].y += balls[i].dy;
  }

  // Render each pixel.
  for (int x = 0; x < maxX; x++) {
    for (int y = 0; y < maxY; y++) {
      float sum = 0;

      // For each pixel, calculate how far all the metaballs are from it to determine if the pixel should be on.
      for (int k = 0; k < numBalls; k++) {
        float dist_squared = (x - balls[k].x) * (x - balls[k].x) + (y - balls[k].y) * (y - balls[k].y);
        if (dist_squared == 0) {
          // If a metaball is exactly on top of the current pixel, it's definitely turning on.
          // Do this just to prevent divide by zero errors.
          sum = pixelActivationThreshold * 2;
        } else {
          sum += (radius * radius) / dist_squared;
        }
      }

      // Turn the pixel on if there were enough metaballs sufficiently close enough to the given pixel.
      if (sum > pixelActivationThreshold) {
        display.drawPixel(true, x, y, false);
      }
    }
  }

  display.refreshEntireDisplay();
}
