#ifndef DEPTHCAMERA_H
#define DEPTHCAMERA_H

// TODO: Delete this include after debugging
#include <time.h>

#include "FlipDotDriver.h"
#include "ControllerInput.h"

class DepthCamera
{
  public:
    DepthCamera(FlipDotDriver& displayDriver);
    void startDepthCamera();
    void renderNextDepthFrame();
    void stopDepthCamera();

  private:
    FlipDotDriver displayDriver;
    clock_t timeOfLastProcessedFrame = 0;
    const int maxRefreshFrameRateHz = 15;
    FILE* depthCamFile = NULL;
    pid_t depthCamPid = 0;
    // 64 * 28 pixels, the resolution the depth camera process returns data at.
    char cameraInputBuffer[1792] = {};

};

#endif // DEPTHCAMERA_H
