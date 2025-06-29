#ifndef LAVALAMPSIM_H
#define LAVALAMPSIM_H

#include "FlipDotDriver.h"
#include "ControllerInput.h"

typedef struct {
  int x;
  int y;
  int dx;
  int dy;
} Ball;

class LavaLampSim
{
  public:
    LavaLampSim(FlipDotDriver& driver);
    void maybeTickSimulation();

  private:
    FlipDotDriver& display;
    int lastSimulationTickTimestamp;
    int maxSimulationHz = 10;
    const static int maxX = 56;
    const static int maxY = 28;
    const static int numBalls = 8;
    constexpr static float radiusIn = 110;
    constexpr static float radius = (radiusIn * radiusIn + (float)(maxX * maxY)) / 15000;//0.9112
    const static int margin = 0;
    // Increase this value for thicker metaballs.
    constexpr static float pixelActivationThreshold = 0.0225;
    Ball balls[numBalls] = {};

    void tickSimulation();
};

#endif // LAVALAMPSIM_H
