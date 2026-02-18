#include "gpu.h"
#include <stdio.h>

// --- THE KERNELS ---
// We embed the OpenCL code here so it's compiled once for the whole system.
const char *SYSTEM_KERNELS = "\
__kernel void hadamard(__global unsigned char* output, const int resolution, const int u, const int v) {\n"
"    int x = get_global_id(0);\n"
"    int y = get_global_id(1);\n"
"    int count = popcount(x & u) + popcount(y & v);\n"
"    output[x + y * resolution] = (count % 2) ? 0 : 255;\n"
"}\n"
"\n"
"__kernel void reconstruct(__global unsigned char* pattern, __global float* image, int sensorVal, int numPixels) {\n"
"    int i = get_global_id(0);\n"
"    if (i < numPixels) {\n"
"        float weight = (pattern[i] > 127) ? 1.0f : -1.0f;\n"
"        image[i] += weight * (float)sensorVal;\n"
"    }\n"
"}";

void checkError(cl_int err, const char *operation) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error during operation '%s': %d\n", operation, err);
        exit(1);
    }
}

GPUContext* gpu_init(const char *kernelSource) {
    GPUContext *gpu = (GPUContext*)malloc(sizeof(GPUContext));
    char deviceName[128];

    // 1. Get Platform & Device
    clGetPlatformIDs(1, &gpu->platform, NULL);
    clGetDeviceIDs(gpu->platform, CL_DEVICE_TYPE_GPU, 1, &gpu->device, NULL);
    
    clGetDeviceInfo(gpu->device, CL_DEVICE_NAME, 128, deviceName, NULL);

    // 2. Create Context
    gpu->context = clCreateContext(NULL, 1, &gpu->device, NULL, NULL, &gpu->err);
    checkError(gpu->err, "clCreateContext");

    // 3. Create Command Queue
    // Note: In OpenCL 2.0+ this signature changes, but this works for 1.2 (Standard)
    gpu->queue = clCreateCommandQueue(gpu->context, gpu->device, 0, &gpu->err);
    checkError(gpu->err, "clCreateCommandQueue");

    // 4. Compile Program
    // If user didn't provide source, use the default embedded one
    const char *src = (kernelSource) ? kernelSource : SYSTEM_KERNELS;
    
    gpu->program = clCreateProgramWithSource(gpu->context, 1, (const char **)&src, NULL, &gpu->err);
    checkError(gpu->err, "clCreateProgramWithSource");

    cl_int buildErr = clBuildProgram(gpu->program, 0, NULL, NULL, NULL, NULL);
    if (buildErr != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(gpu->program, gpu->device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        fprintf(stderr, "Kernel Build Error:\n%s\n", log);
        exit(1);
    }

    return gpu;
}

cl_kernel gpu_create_kernel(GPUContext *gpu, const char *kernelName) {
    cl_int err;
    cl_kernel k = clCreateKernel(gpu->program, kernelName, &err);
    checkError(err, "clCreateKernel");
    return k;
}

cl_mem gpu_create_buffer(GPUContext *gpu, size_t size, void *data, cl_mem_flags flags) {
    cl_int err;
    cl_mem buffer = clCreateBuffer(gpu->context, flags, size, data, &err);
    checkError(err, "clCreateBuffer");
    return buffer;
}

void gpu_free(GPUContext *gpu) {
    if (gpu) {
        clReleaseProgram(gpu->program);
        clReleaseCommandQueue(gpu->queue);
        clReleaseContext(gpu->context);
        free(gpu);
    }
}