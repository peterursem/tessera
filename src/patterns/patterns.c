/*

	patterns.c
	Peter Ursem
	2026
	
	Display hadamard patterns through OpenGL.

*/

#include "patterns.h"
#include "hadamard.frag.h"

#include <stdio.h>	// For printing errors to the console
#include <stdlib.h> // For malloc and free

GLuint shader_program_id;
GLuint patterns_data_texture;
GLuint quad_vao;

// ===== Helper functions =====
int bit_reverse(int n, int resolution);
int dimension_sequency(int index);
int compare_patterns(const void *a, const void *b);

/*

	Compile shader program and set the global program_id

*/
void patterns_shader_init(int resolution)
{
	// Vertex shader for a single fullscreen quad
	const GLchar *vertex_shader_source = "#version 410 core\n"
									   "const vec2 vertices[4] = vec2[4](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));\n"
									   "void main() { gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0); }\n";

	const GLchar *frag_shader_source = (const GLchar *)hadamard_frag;
	GLint frag_shader_length = (GLint)hadamard_frag_len;

	// Shader and program IDs
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program_id = glCreateProgram();

	// Shader compilation debug data
	int success;
    char infoLog[512];

	// Compile vertex shader
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	//Debug
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }

	// Compile fragment shader
	glShaderSource(fragment_shader, 1, &frag_shader_source, &frag_shader_length);
	glCompileShader(fragment_shader);
	// Debug
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }

	// Link program
	glAttachShader(program_id, vertex_shader);
	glAttachShader(program_id, fragment_shader);
	glLinkProgram(program_id);
	// Cleanup
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glUseProgram(program_id);

	shader_program_id = program_id;
}

/*

	Create a list of the patterns to render in sequence order and send it to the GPU
	Returns the flattened list of u, v values

*/
int* patterns_load_sequence(int resolution)
{
	int total_patterns = resolution * resolution;

	// Allocate storage in system memory for patterns
	GPUPattern *patterns = (GPUPattern *)malloc(total_patterns * sizeof(GPUPattern));
	int *patterns_flat = (int *)malloc(total_patterns * 2 * sizeof(int));
	int pattern_id = 0;
	int pattern_u;
	int pattern_v;

	// Find the sequency of each u, v combo
	for (pattern_u = 0; pattern_u < resolution; pattern_u++)
	{
		for (pattern_v = 0; pattern_v < resolution; pattern_v++)
		{
			patterns[pattern_id].u = bit_reverse(pattern_u, resolution);
			patterns[pattern_id].v = bit_reverse(pattern_v, resolution);

			patterns[pattern_id].sequency = dimension_sequency(pattern_u) + dimension_sequency(pattern_v);
			
			pattern_id++;
		}
	}

	// Sort all the patterns
	qsort(patterns, total_patterns, sizeof(GPUPattern), compare_patterns);

	// Unpack the patterns into a an array of {u, v, u, v , u, v } in sequency order.
	for (pattern_id = 0; pattern_id < total_patterns; pattern_id++)
	{
		patterns_flat[pattern_id * 2] = patterns[pattern_id].u;
		patterns_flat[pattern_id * 2 + 1] = patterns[pattern_id].v;
	}

	free(patterns);

	// Create a new texture
	glGenTextures(1, &patterns_data_texture);

	// Bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, patterns_data_texture);

	// Use GL_NEAREST to avoid data loss
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Upload pattern array as a texture with 2 16bit channels
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, resolution, resolution, 0, GL_RG_INTEGER, GL_INT, patterns_flat);

	return patterns_flat;
}

/*

	Initialize OpenGL

*/
int patterns_gl_init()
{
	// Initialize GLFW V4.1 Core
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	return 0;
}

/*

	Initialize the pattern window to fill the height or width of the main window as a square.

*/
GLFWwindow* patterns_window_init() {
	GLFWwindow *window;
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	int resolution;
	int monitor_x;
	int monitor_y;
	int pos_x;
	int pos_y;

	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Removes the window title bar and traffic light buttons
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   // Keeps the window hidden while Ncurses runs

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);			// Match monitor color and refresh specs
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	resolution = mode->width < (mode->height - 100)? mode->width : (mode->height - 100);

	glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
	// Place window in bottom center of the screen
	pos_x = monitor_x + (mode->width - resolution) / 2;
	pos_y = monitor_y + mode->height - resolution;


	// Start window
	window = glfwCreateWindow(resolution, resolution, "Tessera", NULL, NULL);

	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, pos_x, pos_y);

	// Initiaze GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Error: Could not initialize GLAD.\n");
	}

	return window;
}


/*

	Draw and texture the hadamard quad.

*/
void draw_quad()
{
	// Make a vertex array if there isn't one already
	if (quad_vao == 0)
	{
		glGenVertexArrays(1, &quad_vao);
	}

	// Bind the VAO
	glBindVertexArray(quad_vao);

	// Issue the Draw Command to connect
	// 4 vertices into 2 triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Unbind the VAO (cleanup)
	glBindVertexArray(0);
}

/*

	Render a pattern or batch of patterns.
	Put the pattern buffer on screen

*/
int patterns_render(GLFWwindow *window, int batch_start, int batch_size, int resolution, int polarity)
{

	if (glfwWindowShouldClose(window) || batch_start > resolution * resolution) {
		// Terminate
		return -1;
	}

	int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
	
	glUniform1i(glGetUniformLocation(shader_program_id, "batch_start"), batch_start);
	glUniform1i(glGetUniformLocation(shader_program_id, "batch_size"), batch_size);
	glUniform1i(glGetUniformLocation(shader_program_id, "resolution"), resolution);
	glUniform1i(glGetUniformLocation(shader_program_id, "screen_resolution"), fb_height);
	glUniform1i(glGetUniformLocation(shader_program_id, "polarity"), polarity);

	// Draw the fullscreen quad
	draw_quad();

	// Put it on screen
	glfwSwapBuffers(window);
	glfwPollEvents();

	return 0;
}

// ===== Helper Functions =====

/*

	The number of bits in the provided resolution

*/
int bit_depth(int resolution)
{
	int depth = 0;
	int temp = resolution >> 1;
	while (temp > 0)
	{
		temp >>= 1;
		depth++;
	}
	return depth;
}

/*

	Bitwise reverse

*/
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
	return index ^ (index >> 1);
}

/*
	Compare the sequency value of two pattern objects
*/
int compare_patterns(const void *a, const void *b)
{
	GPUPattern *p1 = (GPUPattern *)a;
	GPUPattern *p2 = (GPUPattern *)b;

	return (p1->sequency - p2->sequency);
}