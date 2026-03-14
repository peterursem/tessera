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

// Helper to find the number of active pixels (energy) for a given 1D Haar index
int haar_energy(int index, int resolution) {
    if (index == 0) return resolution;
    
    // Find the scale level (j)
    int j = 0;
    int temp = index;
    while (temp >>= 1) {
        j++;
    }
    // The support shrinks by half for each scale level
    return resolution >> j;
}

// 1D Inverse Fast Haar Transform
void ihaar_1d(float *data, int n) {
    // Temporary array for the reconstruction steps
    float *temp = (float *)malloc(n * sizeof(float));

    // n MUST be a power of 2
    for (int h = 1; h < n; h *= 2) {
        for (int i = 0; i < h; i++) {
            float approx = data[i];
            float detail = data[h + i];

            // Recombine approximation and detail
            temp[2 * i] = approx + detail;
            temp[2 * i + 1] = approx - detail;
        }
        // Copy back the updated approximations for the next scale iteration
        for (int i = 0; i < h * 2; i++) {
            data[i] = temp[i];
        }
    }
    
    free(temp);
}

// 2D Inverse Fast Haar Transform
void ihaar_2d(float *data, int resolution) {
    float *temp_col = (float *)malloc(resolution * sizeof(float));

    // Process all Rows
    for (int row = 0; row < resolution; row++) {
        ihaar_1d(&data[row * resolution], resolution);
    }

    // Process all Columns
    for (int col = 0; col < resolution; col++) {
        // Extract full column
        for (int row = 0; row < resolution; row++) {
            temp_col[row] = data[row * resolution + col];
        }

        // Run the 1D transform on the extracted column
        ihaar_1d(temp_col, resolution);

        // Put the transformed column back into the data buffer
        for (int row = 0; row < resolution; row++) {
            data[row * resolution + col] = temp_col[row];
        }
    }

    free(temp_col);
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
void reconstruct_save(Reconstructor *recon, const char *filename, char mode) {
    float measurement = 0.0f;

    float *transformed = (float *)malloc(recon->total_pixels * sizeof(float));

    if (mode == 'h')
        fwht_2d(recon->measurements, transformed, recon->resolution);
    else {
        for (int u = 0; u < recon->resolution; u++) {
            for (int v = 0; v < recon->resolution; v++) {
                int index = u * recon->resolution + v;
                
                // The 2D energy is the product of the 1D row and column energies
                float energy = (float)(haar_energy(u, recon->resolution) * haar_energy(v, recon->resolution));
                
                transformed[index] = recon->measurements[index] / energy;
            }
        }

        ihaar_2d(transformed, recon->resolution);
    }

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