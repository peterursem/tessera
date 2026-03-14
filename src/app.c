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
#include <math.h>

volatile TesseraStatus app_status;

// === GREEDY ADAPTIVE QUEUE ===
typedef struct {
    int u;
    int v;
    float priority; // <-- New!
} PatternNode;

typedef struct {
    PatternNode *data;
    int head;
    int tail;
    int capacity;
} PatternQueue;

void queue_init(PatternQueue *q, int capacity) {
    q->data = (PatternNode *)malloc(capacity * sizeof(PatternNode));
    q->head = 0;
    q->tail = 0;
    q->capacity = capacity;
}

void queue_push(PatternQueue *q, int u, int v, float priority) {
    if (q->tail < q->capacity) {
        q->data[q->tail].u = u;
        q->data[q->tail].v = v;
        q->data[q->tail].priority = priority;
        q->tail++;
    }
}

PatternNode queue_pop(PatternQueue *q) {
    // Find the pattern with the highest priority in the active queue
    int best_idx = q->head;
    for (int i = q->head + 1; i < q->tail; i++) {
        if (q->data[i].priority > q->data[best_idx].priority) {
            best_idx = i;
        }
    }
    
    // Grab the best node
    PatternNode best_node = q->data[best_idx];
    
    // Swap the best node with the front of the line so we can safely advance 'head'
    q->data[best_idx] = q->data[q->head];
    q->head++;
    
    return best_node;
}

int queue_is_empty(PatternQueue *q) {
    return q->head == q->tail;
}

void queue_free(PatternQueue *q) {
    free(q->data);
}

// Find and queue the children of a given pattern
// Add priority to the arguments
void push_children(PatternQueue *q, int u, int v, int resolution, float priority) {
    int cu[2], cv[2];
    int nu = 0, nv = 0;

    if (u == 0 && v == 0) {
        queue_push(q, 1, 0, priority); 
        queue_push(q, 0, 1, priority); 
        queue_push(q, 1, 1, priority); 
        return;
    }

    if (u == 0) { cu[0] = 0; nu = 1; }
    else { cu[0] = 2 * u; cu[1] = 2 * u + 1; nu = 2; }

    if (v == 0) { cv[0] = 0; nv = 1; }
    else { cv[0] = 2 * v; cv[1] = 2 * v + 1; nv = 2; }

    for (int i = 0; i < nu; i++) {
        for (int j = 0; j < nv; j++) {
            if (cu[i] < resolution && cv[j] < resolution) {
                // Pass priority to the push function
                queue_push(q, cu[i], cv[j], priority);
            }
        }
    }
}

float calculate_noise_threshold(SerialContext *arduino) {
    int samples = 100;
    int readings[samples];
    int sensor_val = 0;
    long sum = 0;
    
    printf("Calibrating sensor noise floor...\n");

    // 1. Gather static samples
    for (int i = 0; i < samples; i++) {
        int reading_status = 0;
        while (!reading_status) {
            reading_status = serial_read_int(arduino, &sensor_val, MIN_SENSOR_READ, MAX_SENSOR_READ);
        }
        readings[i] = sensor_val;
        sum += sensor_val;
    }

    // 2. Calculate the mean (mu)
    float mean = (float)sum / samples;

    // 3. Calculate variance and standard deviation (sigma)
    float variance_sum = 0.0f;
    for (int i = 0; i < samples; i++) {
        float diff = readings[i] - mean;
        variance_sum += (diff * diff);
    }
    
    float std_dev = sqrt(variance_sum / samples);

    // 4. Set threshold to 3.5x standard deviation
    // If your sensor is incredibly clean, enforce a minimum threshold of 1.0
    float threshold = (std_dev * 3.5f) > 1.0f ? (std_dev * 3.5f) : 1.0f;
    
    printf("Noise StdDev: %.2f | Set Threshold: %.2f\n", std_dev, threshold);
    
    return threshold;
}

int main()
{
	// ===== VARS =====

	// Pattern Generation and display
	GLFWwindow *window = NULL;
	int *patterns;
	int pattern_id = 0; // The pattern # to render
	int sign;

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
	app_status.diff = 1;

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
	patterns_shader_init(app_status.resolution, 'w');

	// Load the patterns into VRAM in sequency order and save the sequence
	patterns = patterns_load_sequence(app_status.resolution, 'w');

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

	// ===== The Adaptive Loop =====

    // Create a reverse lookup array to map (u, v) to GPU pattern_id
    int *uv_to_id = (int *)malloc(total_patterns * sizeof(int));
    for (int i = 0; i < total_patterns; i++) {
        uv_to_id[patterns[i * 2] * app_status.resolution + patterns[i * 2 + 1]] = i;
    }

    // Initialize the Queue
    PatternQueue q;
    queue_init(&q, total_patterns);
    
    // 3. Start by queuing ONLY the base approximation pattern with massive priority
    queue_push(&q, 0, 0, 999999.0f);

    int patterns_measured = 0;

    // 4. Run until the queue is completely empty
    while (!queue_is_empty(&q) && app_status.active) {
        
        // Get the next pattern to measure
        PatternNode node = queue_pop(&q);
        int current_u = node.u;
        int current_v = node.v;
        
        // Find the GPU ID for this pattern
        pattern_id = uv_to_id[current_u * app_status.resolution + current_v];

        // --- Standard Measurement Code ---
        for (sign = 1; sign >= -1; sign -= 2) {
            // Note: batch_size is forced to 1 because we need the result NOW to decide what to do next
            if (patterns_render(window, pattern_id, 1, app_status.resolution, sign) < 0) {
                app_status.active = 0; // Trigger shutdown if window closes
                break;
            }

            usleep(1000000 / app_status.framerate);

            if (current_u == 0 && current_v == 0 && sign == 1) {
                sleep(1); // Linger on calibration frame
            }

            while (!reading_status) {
                reading_status = serial_read_int(&arduino, &sensor_val, MIN_SENSOR_READ, MAX_SENSOR_READ);
            }
            reading_status = 0;

            if (current_u == 0 && current_v == 0) {
                reconstruct_calibrate(recon, sensor_val, sign);
            }

            if (!app_status.diff) {
                reconstruct_add(recon, current_u, current_v, sensor_val);
            } else {
                reconstruct_add_diff(recon, current_u, current_v, sensor_val, sign);
            }

            if (!app_status.diff) {
                sign = -2;
            }
        }

        patterns_measured++;
        app_status.progress = patterns_measured;

        // Fetch the measurement we just recorded
        int index = current_u * app_status.resolution + current_v;
        float raw_coeff = fabs(recon->measurements[index]);

        // Unconditionally push the children, using THIS pattern's raw_coeff as their priority!
        push_children(&q, current_u, current_v, app_status.resolution, raw_coeff);

        if (patterns_measured > 0 && patterns_measured % 64 == 0) {
            reconstruct_save_raw(recon, "preview.tsr");
            reconstruct_save(recon, "preview.pgm", 'w');
        }
    }

    // Cleanup Loop variables
    free(uv_to_id);
    queue_free(&q);

	app_status.progress = pattern_id;

	reconstruct_save_raw(recon, "result.tsr");
	reconstruct_save(recon, "result.pgm", 'w');

	free(patterns); // Delete flat packed pattern u, v data
	glfwTerminate(); // End OpenGL

	reconstruct_free(recon); // Delete reconstructor data

	return 0;
}