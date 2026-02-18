#include "reconstruction.h"
#include <stdio.h>
#include <stdlib.h>

Reconstructor* reconstruct_init(GPUContext *gpu, int resolution) {
    Reconstructor *recon = (Reconstructor*)malloc(sizeof(Reconstructor));
    recon->resolution = resolution;
    
    int numPixels = resolution * resolution;
    size_t bufferSize = numPixels * sizeof(float);

    // Get the kernel from the shared GPU context
    recon->kernel = gpu_create_kernel(gpu, "reconstruct");

    // Allocate the Accumulation Buffer (Float) on GPU
    recon->d_accumBuffer = gpu_create_buffer(gpu, bufferSize, NULL, CL_MEM_READ_WRITE);

    // Zero out accumulator
    float *zeros = (float*)calloc(numPixels, sizeof(float));
    clEnqueueWriteBuffer(gpu->queue, recon->d_accumBuffer, CL_TRUE, 0, bufferSize, zeros, 0, NULL, NULL);
    free(zeros);

    // Allocate Host Buffer
    recon->h_resultBuffer = (float*)malloc(bufferSize);

    return recon;
}

void reconstruct_add(GPUContext *gpu, Reconstructor *recon, cl_mem d_patternBuffer, int sensorValue) {
    int numPixels = recon->resolution * recon->resolution;
    size_t globalSize = numPixels;

    // Set Arguments
    // Arg 0: The Pattern (From PatternGenerator)
    clSetKernelArg(recon->kernel, 0, sizeof(cl_mem), &d_patternBuffer);
    
    // Arg 1: The Accumulator (Internal)
    clSetKernelArg(recon->kernel, 1, sizeof(cl_mem), &(recon->d_accumBuffer));
    
    // Arg 2: The Sensor Reading
    clSetKernelArg(recon->kernel, 2, sizeof(int), &sensorValue);
    
    // Arg 3: Size
    clSetKernelArg(recon->kernel, 3, sizeof(int), &numPixels);

    // Add the weighted pattern to the accumulator
    clEnqueueNDRangeKernel(gpu->queue, recon->kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
}

void reconstruct_save(GPUContext *gpu, Reconstructor *recon, const char *filename) {
    int numPixels = recon->resolution * recon->resolution;
    
    // 1. Download from GPU
    clEnqueueReadBuffer(gpu->queue, recon->d_accumBuffer, CL_TRUE, 0, numPixels * sizeof(float), recon->h_resultBuffer, 0, NULL, NULL);

    // 2. Normalize and Save PGM
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    float min = 1e9, max = -1e9;
    for (int i = 1; i < numPixels; i++) {
        if (recon->h_resultBuffer[i] < min) min = recon->h_resultBuffer[i];
        if (recon->h_resultBuffer[i] > max) max = recon->h_resultBuffer[i];
    }
    float range = max - min;
    if (range == 0) range = 1.0;

    fprintf(fp, "P5\n%d %d\n255\n", recon->resolution, recon->resolution);
    for (int i = 0; i < numPixels; i++) {
        unsigned char p = (unsigned char)(((recon->h_resultBuffer[i] - min) / range) * 255.0f);
        fwrite(&p, 1, 1, fp);
    }
    fclose(fp);
}

void reconstruct_free(Reconstructor *recon) {
    if (recon) {
        clReleaseMemObject(recon->d_accumBuffer);
        clReleaseKernel(recon->kernel);
        free(recon->h_resultBuffer);
        free(recon);
    }
}