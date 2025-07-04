// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

/* Include the librealsense C header files */
#include <librealsense2/rs.h>
#include <librealsense2/h/rs_pipeline.h>
#include <librealsense2/h/rs_option.h>
#include <librealsense2/h/rs_frame.h>
#include "example.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     These parameters are reconfigurable                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STREAM          RS2_STREAM_DEPTH  // rs2_stream is a types of data provided by RealSense device           //
#define FORMAT          RS2_FORMAT_Z16    // rs2_format identifies how binary data is encoded within a frame      //
#define WIDTH           640               // Defines the number of columns for each frame or zero for auto resolve//
#define HEIGHT          0                 // Defines the number of lines for each frame or zero for auto resolve  //
#define FPS             30                // Defines the rate of frames per second                                //
#define STREAM_INDEX    0                 // Defines the stream index, used for multiple streams of the same type //
#define HEIGHT_RATIO    17                // Defines the height ratio between the original frame to the new frame //
#define WIDTH_RATIO     10                // Defines the width ratio between the original frame to the new frame  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool shouldQuitProcess = false;
long lastSentFrameTimestampMs = 0;

// The number of meters represented by a single depth unit
float get_depth_unit_value(const rs2_device* const dev) {
	rs2_error* e = 0;
	rs2_sensor_list* sensor_list = rs2_query_sensors(dev, &e);
	check_error(e);

	int num_of_sensors = rs2_get_sensors_count(sensor_list, &e);
	check_error(e);

	float depth_scale = 0;
	int is_depth_sensor_found = 0;
	int i;
	for (i = 0; i < num_of_sensors; ++i) {
		rs2_sensor* sensor = rs2_create_sensor(sensor_list, i, &e);
		check_error(e);

		// Check if the given sensor can be extended to depth sensor interface
		is_depth_sensor_found = rs2_is_sensor_extendable_to(sensor, RS2_EXTENSION_DEPTH_SENSOR, &e);
		check_error(e);

		if (1 == is_depth_sensor_found) {
			depth_scale = rs2_get_option((const rs2_options*)sensor, RS2_OPTION_DEPTH_UNITS, &e);
			check_error(e);
			rs2_delete_sensor(sensor);
			break;
		}
		rs2_delete_sensor(sensor);
	}
	rs2_delete_sensor_list(sensor_list);

	if (0 == is_depth_sensor_found) {
		//printf("Depth sensor not found!\n");
		exit(EXIT_FAILURE);
	}

	return depth_scale;
}

void handle_sigterm(int sig) {
  shouldQuitProcess = true;
}

int main(int argc, char *argv[]) {
  // Provides a mechanism to break out of the infinite loop when the parent process decides to kill this one.
  signal(SIGTERM, handle_sigterm);
  pid_t pidValue = getpid();
  //printf("%d", getpid());
  //fflush(stdout);
	int width = WIDTH;
	int height = HEIGHT;
	int fps = FPS;
	if (argc == 4) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		fps = atoi(argv[3]);
		//printf("Custom resolution specified: %d x %d", width, height);
	}

	rs2_error* e = 0;

	// Create a context object. This object owns the handles to all connected realsense devices.
	// The returned object should be released with rs2_delete_context(...)
	rs2_context* ctx = rs2_create_context(RS2_API_VERSION, &e);
	check_error(e);

	/* Get a list of all the connected devices. */
	// The returned object should be released with rs2_delete_device_list(...)
	rs2_device_list* device_list = rs2_query_devices(ctx, &e);
	check_error(e);

	int dev_count = rs2_get_device_count(device_list, &e);
	check_error(e);
	//printf("There are %d connected RealSense devices.\n", dev_count);
	if (0 == dev_count)
		return EXIT_FAILURE;

	// Get the first connected device
	// The returned object should be released with rs2_delete_device(...)
	rs2_device* dev = rs2_create_device(device_list, 0, &e);
	check_error(e);

	//print_device_info(dev);

	/* Determine depth value corresponding to one meter */
	uint16_t one_meter = (uint16_t)(1.0f / get_depth_unit_value(dev));

	// Create a pipeline to configure, start and stop camera streaming
	// The returned object should be released with rs2_delete_pipeline(...)
	rs2_pipeline* pipeline =  rs2_create_pipeline(ctx, &e);
	check_error(e);

	// Create a config instance, used to specify hardware configuration
	// The retunred object should be released with rs2_delete_config(...)
	rs2_config* config = rs2_create_config(&e);
	check_error(e);

	// Request a specific configuration
	rs2_config_enable_stream(config, STREAM, STREAM_INDEX, width, height, FORMAT, fps, &e);
	check_error(e);

	// Start the pipeline streaming
	// The returned object should be released with rs2_delete_pipeline_profile(...)
	rs2_pipeline_profile* pipeline_profile = rs2_pipeline_start_with_config(pipeline, config, &e);
	if (e) {
		//printf("The connected device doesn't support depth streaming!\n");
		exit(EXIT_FAILURE);
	}

	rs2_stream_profile_list* stream_profile_list = rs2_pipeline_profile_get_streams(pipeline_profile, &e);
	if (e) {
		//printf("Failed to create stream profile list!\n");
		exit(EXIT_FAILURE);
	}

	rs2_stream_profile* stream_profile = (rs2_stream_profile*)rs2_get_stream_profile(stream_profile_list, 0, &e);
	if (e) {
		//printf("Failed to create stream profile!\n");
		exit(EXIT_FAILURE);
	}

	rs2_stream stream;
	rs2_format format;
	int index;
	int unique_id;
	int framerate;
	rs2_get_stream_profile_data(stream_profile, &stream, &format, &index, &unique_id, &framerate, &e);
	if (e) {
		//printf("Failed to get stream profile data!\n");
		exit(EXIT_FAILURE);
	}

	//int width; int height;
	rs2_get_video_stream_resolution(stream_profile, &width, &height, &e);
	if (e) {
		//printf("Failed to get video stream resolution data!\n");
		exit(EXIT_FAILURE);
	}
	int rows = height / HEIGHT_RATIO;
	int row_length = width / WIDTH_RATIO;
	int display_size = (rows * row_length);
	int buffer_size = display_size * sizeof(char);
	char* buffer = calloc(display_size, sizeof(char));
	char* out = NULL;

	while (!shouldQuitProcess) {
		// This call waits until a new composite_frame is available
		// composite_frame holds a set of frames. It is used to prevent frame drops
		// The returned object should be released with rs2_release_frame(...)
		rs2_frame* frames = rs2_pipeline_wait_for_frames(pipeline, RS2_DEFAULT_TIMEOUT, &e);
		check_error(e);

		// Returns the number of frames embedded within the composite frame
		int num_of_frames = rs2_embedded_frames_count(frames, &e);
		check_error(e);

		int i;
		for (i = 0; i < num_of_frames; ++i) {

			// The returned object should be released with rs2_release_frame(...)
			rs2_frame* frame = rs2_extract_frame(frames, i, &e);
			check_error(e);

  		// Restrict the frames to be no more than 10 fps (1/10 of a sec, max refresh rate possible on display).
      /*clock_t currClock = clock();
      double elapsed_time = (currClock - lastSentFrameTimestampMs) / CLOCKS_PER_SEC;
      if (elapsed_time < 0.1) {
				rs2_release_frame(frame);
				continue;
      }
      lastSentFrameTimestampMs = currClock;*/
     // printf("%d\n", currClock);

			// Check if the given frame can be extended to depth frame interface
			// Accept only depth frames and skip other frames
			if (0 == rs2_is_frame_extendable_to(frame, RS2_EXTENSION_DEPTH_FRAME, &e)) {
				rs2_release_frame(frame);
				continue;
			}

			/* Retrieve depth data, configured as 16-bit depth values */
			const uint16_t* depth_frame_data = (const uint16_t*)(rs2_get_frame_data(frame, &e));
			check_error(e);

			/* Print a simple text-based representation of the image, by breaking it into 10x5 pixel regions and approximating the coverage of pixels within one meter */
			out = buffer;
			int x, y, i;
			int* coverage = calloc(row_length, sizeof(int));

			for (y = 0; y < height; ++y) {
				for (x = 0; x < width; ++x) {
					// Create a depth histogram to each row
					int coverage_index = x / WIDTH_RATIO;
					int depth = *depth_frame_data++;
					if (depth > 0 && depth < one_meter)
						++coverage[coverage_index];
				}

				if ((y % HEIGHT_RATIO) == (HEIGHT_RATIO-1)) {
					for (i = 0; i < (row_length); ++i) {
					// TODO: move this to a higher, maybe global scope.
						static const char* pixels = "012345678";//" .:;nhBXW";
						int pixel_index = (coverage[i] / (HEIGHT_RATIO * WIDTH_RATIO / sizeof(pixels)));
						*out++ = pixels[pixel_index];
						coverage[i] = 0;
					}
				//	*out++ = '\n';
				}
			}
			//*out++ = 0;
			// Print 1792 bytes of data to the terminal
      fwrite(&pidValue, sizeof(pid_t), 1, stdout);
			int printStatus = printf("%.1792s", buffer);
			if (printStatus < 0) {
        shouldQuitProcess = true;
			}
			// INCREDIBLY IMPORTANT TO FLUSH THIS!!
			// IT MAKES SURE THE PARENT PROCESS READING THE DATA DOESN'T GET PARTIAL READS!
			fflush(stdout);

			free(coverage);
			rs2_release_frame(frame);
		}

		rs2_release_frame(frames);
		if (shouldQuitProcess) {
      break;
		}
	}

	// Stop the pipeline streaming
	rs2_pipeline_stop(pipeline, &e);
	check_error(e);

	// Release resources
	free(buffer);
	rs2_delete_pipeline_profile(pipeline_profile);
	rs2_delete_stream_profiles_list(stream_profile_list);
	rs2_delete_stream_profile(stream_profile);
	rs2_delete_config(config);
	rs2_delete_pipeline(pipeline);
	rs2_delete_device(dev);
	rs2_delete_device_list(device_list);
	rs2_delete_context(ctx);

	return EXIT_SUCCESS;
}
