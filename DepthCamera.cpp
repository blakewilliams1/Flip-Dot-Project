#include "DepthCamera.h"

#include <iostream>
#include <csignal>
#include <fcntl.h>

//#include "FlipDotDriver.h"
//#include "ControllerInput.h"
using namespace std;

DepthCamera::DepthCamera(FlipDotDriver& _display) : displayDriver(_display) {}

void DepthCamera::startDepthCamera() {
  displayDriver.clearDisplay();
  displayDriver.drawText("loading", 15, 12);
  displayDriver.refreshEntireDisplay();

  depthCamFile = popen("./rs-depth", "r");
  if (!depthCamFile) {
    cout << "Error starting depth camera!" << endl;
    return;
  }

  // Read the PID of the newly created child process before setting reads to non-blocking.
  // We need to know the PID to give it the kill signal when exiting from depth cam state.
  fread(&depthCamPid, sizeof(pid_t), 1, depthCamFile);

  // Set the file descriptor to be nonblocking reads.
  int depthCamFd = fileno(depthCamFile);
  int depthCamFdFlags = fcntl(depthCamFd, F_GETFL, 0);
  fcntl(depthCamFd, F_SETFL, depthCamFdFlags | O_NONBLOCK);

  cout << "starting depth camera!" << endl;
}

void DepthCamera::stopDepthCamera() {
  // Shut down the depth cam child process we have a reference to it (implies it's live still).
  if (depthCamPid > 0) {
    kill(depthCamPid, SIGTERM);
    depthCamPid = -1;
    depthCamFile = nullptr;
  }
}

void DepthCamera::renderNextDepthFrame() {
	// Read depth camera data. All values > 2 are on pixels. Incoming buffer will be 64 x 28 pixels
	// but the first 5 values of each row should be trimmed off becaues they're dead for some reason.
	// The actual panel 56 x 28 in size.
	size_t cameraReadStatus =
	  fread(&cameraInputBuffer, sizeof(char), sizeof(cameraInputBuffer), depthCamFile);

	// Not bytes read this pass.
	if (cameraReadStatus == 0) {
    return;
	}

	// Bytes have been read, and we're assuming that's a full frame of data! We're recieving at 30fps but present at
	// whatever the fastest framerate I can get by with. Initial tests showed that to be about 10fps for full screen
	// refreshes, but in practice the depth cam data does change every single pixel at the same time and we can bump
	// it up a little more.
  clock_t currClock = clock();
  double elapsedTimeSinceLastProcessedFrameSec = (double)(currClock - timeOfLastProcessedFrame) / CLOCKS_PER_SEC;
  if (elapsedTimeSinceLastProcessedFrameSec < ((double)1 / maxRefreshFrameRateHz)) {
    return;
  }
  // The size of the display in pixels, divided by 8 because each element of this array holds 8 pixels of data.
  byte rawDisplayBuffer[(56 * 28) / 8] = {};
  timeOfLastProcessedFrame = currClock;
  // There are 28 rows on the display, process them one at a time.
  int cameraBufferRowWidth = 64;
  // The nested for loops are designed to iterate over the desired elements of the camera data, but that is not aligned
  // with the indexing of the output buffer for displays. Use this index variable to track where we should put the
  // processed data within the display buffer.
  int currDisplayPixelIndex = 0;
  for (int currRow = 0; currRow < 28; currRow++) {
    for (int currPixel = 8; currPixel < cameraBufferRowWidth; currPixel++) {
      // Get the data at the camera pixel.
      int currDepthValue = cameraInputBuffer[(currRow * cameraBufferRowWidth) + currPixel] - '0';
      bool isPixelOn = currDepthValue > 1;
      // Set the data into the display input buffer.
      byte relevanteByte = rawDisplayBuffer[currDisplayPixelIndex / 8];
      if (isPixelOn) {
        // Force bit to be high.
        relevanteByte |= 1 << (currDisplayPixelIndex % 8);
      }
      // Save the modification back into the display buffer and increment the index of which bit to set next.
      rawDisplayBuffer[currDisplayPixelIndex / 8] = relevanteByte;
      currDisplayPixelIndex++;
    }
  }
  //printf("%.1792s\n", cameraInputBuffer);
  //cout << "going to process: " << cameraReadStatus << " bytes. elapsedTimeSinceLastProcessedFrame: " << elapsedTimeSinceLastProcessedFrameSec << endl;
  displayDriver.clearDisplay();
  displayDriver.setRawDisplayData(rawDisplayBuffer);
  displayDriver.refreshEntireDisplay();
}
