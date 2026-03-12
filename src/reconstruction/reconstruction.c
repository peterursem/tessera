#include "reconstruction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void fwht(const float *input, float *output, int n) {
    // Copy to leave the input untouched
    memcpy(output, input, n * sizeof(float));

    // n MUST be a power of 2 (e.g., 4096, 16384, etc.)
    for (int h = 1; h < n; h *= 2) {
        for (int i = 0; i < n; i += h * 2) {
            for (int j = i; j < i + h; j++) {
                float x = output[j];
                float y = output[j + h];

                // In-place butterfly
                output[j] = x + y;
                output[j + h] = x - y;
            }
        }
    }
}

void fwht_2d(const float *input, float *output, int resolution) {
    // Allocate temporary buffers for column extraction
    float *temp_col_in = (float *)malloc(resolution * sizeof(float));
    float *temp_col_out = (float *)malloc(resolution * sizeof(float));

    // Process all Rows
    for (int row = 0; row < resolution; row++) {
        fwht(&input[row * resolution], &output[row * resolution], resolution);
    }

    // Process all Columns
    for (int col = 0; col < resolution; col++) {
        // Get full column
        for (int row = 0; row < resolution; row++) {
            temp_col_in[row] = output[row * resolution + col];
        }

        // Run the 1D transform on the extracted column
        fwht(temp_col_in, temp_col_out, resolution);

        // Put the transformed column back into the output buffer
        for (int row = 0; row < resolution; row++) {
            output[row * resolution + col] = temp_col_out[row];
        }
    }

    // Clean up
    free(temp_col_in);
    free(temp_col_out);
}

Reconstructor* reconstruct_init(int resolution) {
    Reconstructor *recon = (Reconstructor*)malloc(sizeof(Reconstructor));
    int total_pixels = resolution * resolution;

    recon->resolution = resolution;
    recon->total_pixels = total_pixels;
    recon->average = 0.0f;
    recon->measurements = (float *)calloc(total_pixels, sizeof(float));

    return recon;
}

/*

  Set the full brightness measurement average  

*/
void reconstruct_calibrate(Reconstructor *recon, int sensor_value, int sign) {
    if (sign > 0) {
        recon->average += sensor_value;
    } else {
        recon->average -= sensor_value;
    }
}

/*

    Add a sensor value to the measurement array

*/
void reconstruct_add(Reconstructor *recon, int pattern_u, int pattern_v, int sensor_value) {
    int pattern_index = pattern_u * recon->resolution + pattern_v;
    recon->measurements[pattern_index] = (2 * sensor_value) - recon->average;
}

void reconstruct_add_diff(Reconstructor *recon, int pattern_u, int pattern_v, int sensor_value, int sign) {
    int pattern_index = pattern_u * recon->resolution + pattern_v;
    if (sign > 0) {
        recon->measurements[pattern_index] += sensor_value;
    } else {
        recon->measurements[pattern_index] -= sensor_value;
    }
}

void reconstruct_save_raw(Reconstructor *recon, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening raw file %s\n", filename);
        return;
    }

    fwrite(&recon->resolution, sizeof(int), 1, fp);
    fwrite(&recon->average, sizeof(float), 1, fp);

    fwrite(recon->measurements, sizeof(float), recon->total_pixels, fp);

    fclose(fp);
}

/*

    Normalize the output buffer and save it

*/
void reconstruct_save(Reconstructor *recon, const char *filename) {
    float measurement = 0.0f;

    float *transformed = (float *)malloc(recon->total_pixels * sizeof(float));

    fwht_2d(recon->measurements, transformed, recon->resolution);


    // Find min / max
    float min = 1e9, max = -1e9;
    for (int px = 1; px < recon->total_pixels; px++) {
        if (transformed[px] < min) min = transformed[px];
        if (transformed[px] > max) max = transformed[px];
    }

    // Calculate range
    float range = max - min;
    if (range == 0) range = 1.0;

    // Open the file
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    // Output .PGM image
    fprintf(fp, "P5\n%d %d\n255\n", recon->resolution, recon->resolution);
    for (int px = 0; px < recon->total_pixels; px++) {
        // Normalize the pixel luminance
        float value = ((transformed[px] - min) / range) * 255.0f;

        // Clamp to 0-255 range
        if (value < 0.0f) value = 0.0f;
        if (value > 255.0f) value = 255.0f;

        // Add to image
        unsigned char p = (unsigned char)value;
        fwrite(&p, 1, 1, fp);
    }

    free(transformed);
    // Close the file
    fclose(fp);
}

void reconstruct_free(Reconstructor *recon) {
    if (recon) {
        free(recon->measurements);
        free(recon);
    }
}