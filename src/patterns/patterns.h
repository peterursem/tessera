#ifndef PATTERNS_H
#define PATTERNS_H

#include <glad/glad.h>  // OpenGL needs to be defined to use pattern functions
#include <GLFW/glfw3.h>

// The logical representation of a single pattern
typedef struct {
    int u;
    int v;
    int sequency;
} GPUPattern;


// Initialize the pattern window
GLFWwindow* patterns_window_init();

// Initialize shader program
void patterns_shader_init(int resolution);

// Initialize OpenGL
int patterns_gl_init();

// Load a pattern sequence into VRAM and return the sequence
int* patterns_load_sequence(int resolution);

// Render specific pattern or a batch on the GPU
int patterns_render(GLFWwindow *window, int batch_start, int batch_size, int resolution);

#endif