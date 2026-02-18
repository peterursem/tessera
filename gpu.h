#ifndef GPU_H
#define GPU_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_int err;
} GPUContext;

// Initialize OpenCL
GPUContext* gpu_init(const char *kernelSource);

// Create a kernel
cl_kernel gpu_create_kernel(GPUContext *gpu, const char *kernelName);

// Allocate a buffer
cl_mem gpu_create_buffer(GPUContext *gpu, size_t size, void *data, cl_mem_flags flags);

// Cleanup
void gpu_free(GPUContext *gpu);

#endif