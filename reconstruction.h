#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "gpu.h"

typedef struct {
    int resolution;
    
    cl_kernel kernel;           // Handle to the "reconstruct" kernel
    cl_mem d_accumBuffer;       // The floating point image accumulator on GPU
    float *h_resultBuffer;      // Host buffer for saving/viewing
} Reconstructor;

// Initialize the Reconstructor (Linking it to the GPU Context)
Reconstructor* reconstruct_init(GPUContext *gpu, int resolution);

// Add a measurement
// 'd_patternBuffer' comes from the PatternGenerator!
void reconstruct_add(GPUContext *gpu, Reconstructor *r, cl_mem d_patternBuffer, int sensorValue);

// Save current state to PGM file
void reconstruct_save(GPUContext *gpu, Reconstructor *r, const char *filename);

// Cleanup
void reconstruct_free(Reconstructor *r);

#endif