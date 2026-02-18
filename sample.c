#include "sample.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Initialize the serial port in NON-BLOCKING mode
int serial_init(SerialContext* ctx, const char* port_name, int baud_rate) {
    ctx->buf_pos = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));

    // Open with O_NONBLOCK (Critical for use in loops)
    ctx->fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    
    if (ctx->fd < 0) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios tty;
    if (tcgetattr(ctx->fd, &tty) != 0) return -1;

    // Set Baud Rate
    speed_t speed = B9600; // Default
    if (baud_rate == 115200) speed = B115200;
    // Add other speeds if necessary
    
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // Make Raw and Non-Blocking
    cfmakeraw(&tty);
    tty.c_cc[VMIN] = 0;  // Return immediately even if 0 bytes
    tty.c_cc[VTIME] = 0; // No timeout

    if (tcsetattr(ctx->fd, TCSANOW, &tty) != 0) return -1;
    
    tcflush(ctx->fd, TCIFLUSH);
    return 0;
}

// Returns: 1 if a new integer was parsed, 0 if no new data yet.
int serial_read_int(SerialContext* ctx, int* value_out) {
    tcflush(ctx->fd, TCIFLUSH); // Flush out old values before reading

    char trigger = 'r';
    write(ctx->fd, &trigger, 1);

    //usleep(syncDelay); // MIGHT NEED TO CHANGE, DELAY TO FILL THE BUFFER AT 10HZ

    char temp_buf[64];
    
    // Attempt to read whatever is currently available
    int n = read(ctx->fd, temp_buf, sizeof(temp_buf));
    
    if (n > 0) {
        // Append new bytes to our persistent context buffer
        for (int i = 0; i < n; i++) {
            char c = temp_buf[i];
            
            // If newline, we have a complete number
            if (c == '\n' || c == '\r') {
                if (ctx->buf_pos > 0) {
                    ctx->buffer[ctx->buf_pos] = '\0'; // Null terminate
                    *value_out = atoi(ctx->buffer);   // Convert
                    ctx->buf_pos = 0;                 // Reset buffer
                    if(*value_out > 1024 || *value_out < 0) {
                        //Redo if output is invalid
                        return serial_read_int(ctx, value_out);
                    }
                    return 1;                         // Signal success
                }
            } 
            // Otherwise, keep buffering
            else if (ctx->buf_pos < sizeof(ctx->buffer) - 1) {
                ctx->buffer[ctx->buf_pos++] = c;
            }
        }
    }
    
    return 0; // No complete number ready yet
}

void serial_close(SerialContext* ctx) {
    close(ctx->fd);
}