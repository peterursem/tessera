#include "patterns.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

// Helper functions
int bit_depth(int resolution) {
    int depth = 0;
    int temp = resolution >> 1;
    while (temp > 0) {
        temp >>= 1;
        depth++;
    }
    return depth;
}

int bit_reverse(int n, int resolution)
{
	int reversed = 0;
	int i;

	for (i = 0; i < bit_depth(resolution); i++)
	{
		reversed = (reversed << 1) | (n & 1);
		n >>= 1;
	}

	return reversed;
}

/*
	Determine the sequency value of one dimension of a pattern
*/
int dimension_sequency(int index)
{
	return index ^ index >> 1;
}

/*
	Compare the sequency value of two pattern objects
*/
int compare_patterns(const void *a, const void *b)
{
	Pattern *p1 = (Pattern *)a;
	Pattern *p2 = (Pattern *)b;

	return (p1->sequency - p2->sequency);
}

PatternGenerator* patterns_init(GPUContext *gpu, int resolution) {
	PatternGenerator *generator = (PatternGenerator *)malloc(sizeof(PatternGenerator));

	generator->resolution = resolution;
	generator->totalPatterns = resolution * resolution;
	generator->globalSize[0] = resolution;
	generator->globalSize[1] = resolution;

	generator->kernel = gpu_create_kernel(gpu, "hadamard");
	
	generator->d_buffer = gpu_create_buffer(gpu, generator->totalPatterns * sizeof(unsigned char), NULL, CL_MEM_READ_WRITE);

	// Arg 0: Output Buffer
    // Arg 1: Resolution
    clSetKernelArg(generator->kernel, 0, sizeof(cl_mem), &(generator->d_buffer));
    clSetKernelArg(generator->kernel, 1, sizeof(int), &(generator->resolution));

	return generator;
}

Pattern* patterns_create_sequence(int resolution) {

	// Allocate storage for patterns
	Pattern *patterns = (Pattern *)malloc(resolution * resolution * sizeof(Pattern));
	int patternId = 0;
	int patternU;
	int patternV;

	for (patternU = 0; patternU < resolution; patternU++) {
		for (patternV = 0; patternV < resolution; patternV++) {
			patterns[patternId].u = bit_reverse(patternU, resolution);
			patterns[patternId].v = bit_reverse(patternV, resolution);

			patterns[patternId].sequency = dimension_sequency(patternU) + dimension_sequency(patternV);
			patternId++;
		}
	}

	qsort(patterns, resolution * resolution, sizeof(Pattern), compare_patterns);

	return patterns;
}

void patterns_render(GPUContext *gpu, PatternGenerator *generator, Pattern pattern, unsigned char *h_buffer) {
	clSetKernelArg(generator->kernel, 2, sizeof(int), &(pattern.u));
	clSetKernelArg(generator->kernel, 3, sizeof(int), &(pattern.v));

	clEnqueueNDRangeKernel(gpu->queue, generator->kernel, 2, NULL, generator->globalSize, NULL, 0, NULL, NULL);

	if (h_buffer != NULL) {
		clEnqueueReadBuffer(gpu->queue, generator->d_buffer, CL_TRUE, 0, generator->resolution * generator->resolution, h_buffer, 0, NULL, NULL);
	}
}

void patterns_free(PatternGenerator *generator) {
    if (generator) {
        clReleaseMemObject(generator->d_buffer);
        clReleaseKernel(generator->kernel);
        free(generator);
    }
}