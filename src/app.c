/*

	app.c
	Peter Ursem
	2026

	Manage the patterning, sampling, and reconstruction

*/

#include "app.h"

#include "sample/sample.h"
#include "patterns/patterns.h"
#include "reconstruction/reconstruction.h"
#include "ui/ui.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile TesseraStatus app_status;

int main()
{
	// ===== VARS =====

	// Pattern Generation and display
	GLFWwindow *window = NULL;
	int *patterns;
	int pattern_id = 0; // The pattern # to render

	// Sampling 
	SerialContext arduino; // The serial communication context
	int sensor_val = 0;
	int reading_status = 0;

	// Reconstruction
	Reconstructor *recon;

	// UI
	pthread_t tui_thread;

	// Control
	int total_patterns;

	// ===== Setup =====

	app_status.active = 0;
    app_status.batch_size = 1;
    app_status.progress = 0.0f;

	// Start Terminal UI
	if(!pthread_create(&tui_thread, NULL, open_ui, NULL)) {
		fprintf(stderr, "Couldn't start TUI thread");
		//return -1;
	}

	while(app_status.active == 0) {
		usleep(300000);
	}

	// ===== Start the OpenGL program =====
	
	total_patterns = app_status.resolution * app_status.resolution;

	// Init OpenGL
	patterns_gl_init();
	// Start a borderless fulscreen square window
	window = patterns_window_init();
	// Load the shader
	patterns_shader_init(app_status.resolution);

	// Load the patterns into VRAM in sequency order and save the sequence
	patterns = patterns_load_sequence(app_status.resolution);

	// Setup serial communication
	if (serial_init(&arduino, (char *)app_status.sensor_port) < 0)
	{
		fprintf(stderr, "Error: Could not open serial port.\n");
		return -1;
	}

	//Setup Reconstructor
	recon = reconstruct_init(app_status.resolution);

	// Show the window
	glfwShowWindow(window);

	// ===== The Loop =====

	for (pattern_id = 0; pattern_id < total_patterns; pattern_id += app_status.batch_size)
	{
		// Put pattern on screen
		if (patterns_render(window, pattern_id, app_status.batch_size, app_status.resolution) < 0) {
			break;
		}

		usleep(1000000 / app_status.framerate);

		if (pattern_id == 0)
		{
			// Linger on the calibration frame for one second
			sleep(1);
		}

		while (!reading_status)
		{
			reading_status = serial_read_int(&arduino, &sensor_val, MIN_SENSOR_READ, MAX_SENSOR_READ);
		}
		reading_status = 0;

		if (pattern_id == 0) {
			// Take average from first pattern (100% White)
			reconstruct_calibrate(recon, sensor_val);
		}
		
		// Add to the measurement matrix at the given u, v index
		reconstruct_add(recon, patterns[pattern_id * 2], patterns[pattern_id * 2 + 1], sensor_val);

		// Render TUI
		app_status.progress = pattern_id;

		if (pattern_id > 0 && pattern_id % (total_patterns / 100) == 0)
		{
			reconstruct_save_raw(recon, "preview.bin");
			reconstruct_save(recon, "preview.pgm");
		}

	}

	app_status.progress = pattern_id;

	reconstruct_save_raw(recon, "result.bin");
	reconstruct_save(recon, "result.pgm");

	free(patterns); // Delete flat packed pattern u, v data
	glfwTerminate(); // End OpenGL

	reconstruct_free(recon); // Delete reconstructor data

	return 0;
}