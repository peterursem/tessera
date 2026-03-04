#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

typedef struct {
    int resolution;
    int total_pixels;
    float average;          // The average sample value
    float *measurements;    // An array of all samples in natural order
} Reconstructor;

// Initialize the Reconstructor (Linking it to the GPU Context)
Reconstructor* reconstruct_init(int resolution);

// Measure a calibration frame
void reconstruct_calibrate(Reconstructor *recon, int sensor_value);

// Add a measurement to the array
void reconstruct_add(Reconstructor *recon, int pattern_u, int pattern_v, int sensor_value);

// Save current image to PGM file
void reconstruct_save(Reconstructor *recon, const char *filename);

// Save current measurements to BIN file
void reconstruct_save_raw(Reconstructor *recon, const char *filename);

// Cleanup
void reconstruct_free(Reconstructor *r);

#endif