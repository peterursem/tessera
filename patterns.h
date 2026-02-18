#ifndef PATTERNS_H
#define PATTERNS_H

#include "gpu.h"

// The logical representation of a single pattern
typedef struct {
    int u;
    int v;
    int sequency;
} Pattern;

// The Generator Module
typedef struct {
    int resolution;
    int totalPatterns;
    size_t globalSize[2]; // [width, height]
    
    cl_kernel kernel;     // Handle to the "hadamard" kernel
    cl_mem d_buffer;      // The Output Buffer on GPU (0-255)
} PatternGenerator;

// Initialize the generator (Linking it to the GPU Context)
PatternGenerator* patterns_init(GPUContext *gpu, int resolution);

// Generate the list of Walsh-ordered patterns (CPU side)
Pattern* patterns_create_sequence(int resolution);

// Render specific pattern on GPU
// If 'h_dest' is not NULL, it reads the result back to CPU (for FFMPEG)
void patterns_render(GPUContext *gpu, PatternGenerator *pg, Pattern p, unsigned char *h_dest);

// Cleanup
void patterns_free(PatternGenerator *pg);

#endif