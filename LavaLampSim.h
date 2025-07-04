#ifndef LAVALAMPSIM_H
#define LAVALAMPSIM_H

#include "FlipDotDriver.h"
#include "ControllerInput.h"

typedef struct {
  float x;
  float y;
  float dx;
  float dy;
} Ball;

class LavaLampSim
{
  public:
    LavaLampSim(FlipDotDriver& driver);
    void maybeTickSimulation(CONTROLLER_INPUT controllerValue, bool wasPressed);

  private:
    FlipDotDriver& display;
    int lastSimulationTickTimestamp;
    int maxSimulationHz = 7;
    const static int maxX = 56;
    const static int maxY = 28;
    const static int numBalls = 10;
    const static int numNegativeBalls = 3;
    constexpr static float radiusIn = 110;
    constexpr static float radius = 0.9;//(radiusIn * radiusIn + (float)(maxX * maxY)) / 15000;//0.9112
    // This allows the balls to bounce slightly out of bounds to improve the look.
    const static int margin = -5;
    // Increase this value for thicker metaballs.
    constexpr static float pixelActivationThreshold = 0.0225;
    Ball balls[numBalls] = {};
    Ball negativeBalls[numNegativeBalls] = {};

    void tickSimulation();
    void resetDirections();
    void simulateBall(Ball& ball);
    float calculateDistSquared(Ball& ball, int x, int y);
    void resetBallState(Ball& ball);
};

#endif // LAVALAMPSIM_H
