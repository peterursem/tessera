#include "manager.h"

#include "sample.h"
#include "patterns.h"
#include "reconstruction.h"
#include "app.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int resolution = 64;
int framerate = 4;
volatile int progress = 0;
char sensor_port[32];

// Helper: Fisher-Yates Shuffle to randomize pixels efficiently
void shuffle_pixels(unsigned char *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        unsigned char temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

float calibrate(GPUContext *gpu, FILE *pattern_pipe, SerialContext *arduino, int distinct_frames, int resolution) {  
	int readingStatus = 0;  
	int sensorVal = 0;
    long total_reading = 0;
    int num_pixels = resolution * resolution;
    unsigned char *calib_pattern = (unsigned char*)malloc(num_pixels);

    // 1. Prepare a perfectly balanced (50/50) base array
    for (int i = 0; i < num_pixels; i++) {
        calib_pattern[i] = (i < num_pixels / 2) ? 0 : 255;
    }

    // 2. Loop through multiple distinct random patterns
    for (int k = 0; k < distinct_frames; k++) {
        
        // Randomize pixel positions
        shuffle_pixels(calib_pattern, num_pixels);

        // Display
        fwrite(calib_pattern, 1, num_pixels, pattern_pipe);
        fflush(pattern_pipe);
        
        // Wait for sensor
        usleep(1000000 / framerate);

        while (!readingStatus) {
			readingStatus = serial_read_int(arduino, &sensorVal);
		}
		readingStatus = 0;
        
        total_reading += sensorVal;
    }
    
    free(calib_pattern);

    // 3. Return the average
    return (float)total_reading / distinct_frames;
}

void* app_main()
{
	int calibrationSensings = 50;

	// Shared GPU context
	GPUContext *gpu;			 	// The OpenCL GPU context

	// Pattern Generation and display
	PatternGenerator *patterner;
	Pattern *sequence;
	int patternId = 0; 				// The pattern # to render
	size_t frameSize = resolution * resolution * sizeof(unsigned char);
	unsigned char *patternBuffer = (unsigned char *)malloc(frameSize); 	// The pattern host buffer
	// Pattern display
	char ffplay_cmd[400];
	FILE *pattern_pipe;

	// Sampling and Reconstruction
	SerialContext arduino;		 	// The serial communication context
	int readingStatus = 0;
	int sensorVal = 0;
	float readingAverage = 0.0f;
	float weightedVal;
	Reconstructor *recon;
	
	// Setup serial communication
	if (serial_init(&arduino, sensor_port, 9600) < 0) {
        fprintf(stderr, "Error: Could not open serial port.\n");
        return NULL;
    }

	gpu = gpu_init(NULL);
    if (!gpu) {
        fprintf(stderr, "Error: Could not initialize OpenCL.\n");
        return NULL;
    }

	// Setup pattern generator
	patterner = patterns_init(gpu, resolution);
	recon = reconstruct_init(gpu, resolution);
	// Order the patterns
	sequence = patterns_create_sequence(resolution);
	// Setup pattern output pipe
	snprintf(ffplay_cmd, sizeof(ffplay_cmd), 
		"ffplay "
		"-f rawvideo "
		"-pixel_format gray "
		"-video_size %dx%d "
		"-framerate %d "
		"-fflags nobuffer "       // Disable input buffer
		"-flags low_delay "       // Force low-delay decoding
		"-framedrop "             // Drop frames if rendering lags (keeps real-time sync)
		"-probesize 32 "          // Stop ffplay from analyzing initial data
		"-analyzeduration 0 "     // Reduce startup analysis time to zero
		"-sync ext "              // Sync to external clock (the incoming pipe stream)
		"-an "                    // Explicitly disable audio to prevent A/V sync waits
		"-vf 'scale=1440:1440:force_original_aspect_ratio=decrease:flags=neighbor,pad=1440:1440:-1:-1:color=black' "
		"-left 5120 -top 1000 "
		"-loglevel quiet "
		"-nostats "
		"-hide_banner "
		"-noborder "
		"-i -", 
		resolution, resolution, framerate);

	pattern_pipe = popen(ffplay_cmd, "w");
	if (!pattern_pipe) {
		fprintf(stderr, "Error: Could not launch ffplay.\n");
        return NULL;
	}

	sleep(1);

	readingAverage = calibrate(gpu, pattern_pipe, &arduino, 50, resolution);


	// The loop
	for (patternId = 0; patternId < patterner->totalPatterns; patternId++)
	{
		patterns_render(gpu, patterner, sequence[patternId], patternBuffer);

		fwrite(patternBuffer, 1, frameSize, pattern_pipe);
		fflush(pattern_pipe);

		usleep(1000000 / framerate);
		if (patternId == 1) {
			sleep(1);
		}
		while (!readingStatus) {
			readingStatus = serial_read_int(&arduino, &sensorVal);
		}
		readingStatus = 0;

		// Average out the readings (bar the first).
		if(patternId > 0) {
			readingAverage = (readingAverage * 0.95f) + ((float)sensorVal * 0.05f);
		}

		weightedVal = (float)sensorVal - readingAverage;
		reconstruct_add(gpu, recon, patterner->d_buffer, weightedVal);

        progress = patternId;
        
        if (patternId > 0 && patternId % 16 == 0) {
            reconstruct_save(gpu, recon, "out/preview.pgm");
        }
	}

	pclose(pattern_pipe);
	free(patternBuffer);

	reconstruct_save(gpu, recon, "out/result.pgm");

	return NULL;
}