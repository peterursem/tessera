/*

	hadamard.frag
	Peter Ursem
	2026

	Calculate the hadamard pixel values for up to 24 patterns at once.
	If the batch size is greater than one, patterns will be bit-packed into a 24bit color image for DLP boards.
	If the batch sixr is one, patterns will be displayed individually in monochrome.

*/

#version 410 core

#define MAX_BATCH 24

out vec4 frag_colour;

// Uniform: User set but constant within the shader
uniform int batch_start;
uniform int batch_size;
uniform int screen_resolution;
uniform int resolution;
uniform int polarity;
uniform isampler2D pattern_data;

// Hadamard calculation
bool hadamard_pixel(int x, int y, int u, int v) {
    // Bitwise AND the coordinate with the frequency, XOR them, count bits.
    // If the count is even: pixel = +1, odd: pixel = -1
    int bits = (x & u) ^ (y & v);

    if (polarity > 0)
        return (bitCount(bits) & 1) == 0;
    else
        return (bitCount(bits) & 1) == 1;
}

/*

    Output the value of one pixel of a Hadamard matrix.

    If the batch size is greater than one the bits will be packed into colour channels

*/
void main() {
    // Get the pixel coordinate in the pattern by finding the relative positon on screen
    int x = int(float(resolution) * gl_FragCoord.x / screen_resolution);
    int y = int(float(resolution) * gl_FragCoord.y / screen_resolution);

    uint packed_bits = 0u;

    // Calculate a batch of values for this pixel
    for (int pat_offset = 0; pat_offset < min(batch_size, MAX_BATCH); pat_offset++) {
        int pat_id = batch_start + pat_offset;
        
        // Get pattern definition from VRAM
        ivec2 tex_pos = ivec2(pat_id % resolution, pat_id / resolution);
        ivec2 pat = texelFetch(pattern_data, tex_pos, 0).rg;

        // Compute Hadamard pixel value
        bool val = hadamard_pixel(x, y, pat.x, pat.y);

        // Merge in a bit at the specified location
        if (batch_size == 1) {
            // ===== Monitor Mode =====
            // All white pixel
            packed_bits = val? 0xFFFFFF : 0;
        } else if (val) {
            // ===== DLP Mode =====
            // Set only the bit at the pattern offset index
            packed_bits |= (1u << pat_offset);
        }
    }

    // Create output colour
    // For DLPs each bit represents a pattern
    // For monitors the pixel will be white or black
    float r = float((packed_bits >> 0)  & 0xFFu) / 255.0;
    float g = float((packed_bits >> 8)  & 0xFFu) / 255.0;
    float b = float((packed_bits >> 16) & 0xFFu) / 255.0;

    // Return the pixel value
    frag_colour = vec4(r, g, b, 1.0);
}