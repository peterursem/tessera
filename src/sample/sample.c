#include "sample.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

/*

	Initialize the serial port in a non-blocking mode.

*/
int serial_init(SerialContext *ctx, const char *port_name, int baud_rate)
{
	ctx->buf_pos = 0;
	memset(ctx->buffer, 0, sizeof(ctx->buffer));

	// Open with O_NONBLOCK
	ctx->fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (ctx->fd < 0)
	{
		perror("Error opening serial port");
		return -1;
	}

	struct termios tty;
	if (tcgetattr(ctx->fd, &tty) != 0)
		return -1;

	// Set Baud Rate
	speed_t speed = B9600;
	if (baud_rate == 115200)
		speed = B115200;

	cfsetispeed(&tty, speed);
	cfsetospeed(&tty, speed);

	cfmakeraw(&tty);	 // Make Raw and Non-Blocking
	tty.c_cc[VMIN] = 0;	 // Return immediately even if 0 bytes
	tty.c_cc[VTIME] = 0; // No timeout

	// Apply settings
	if (tcsetattr(ctx->fd, TCSANOW, &tty) != 0)
		return -1;

	tcflush(ctx->fd, TCIFLUSH);
	return 0;
}

/*

	Read the last int passed over serial.

	Returns 0 on a successful read.

*/
int serial_read_int(SerialContext *ctx, int *value_out, int min_val, int max_val) {
    // 1. Flush and trigger EXACTLY ONCE per measurement
    tcflush(ctx->fd, TCIFLUSH); // Flush out old values before reading

	// Send a trigger to the arduino to request a measurement.
    char trigger = 'r';
    write(ctx->fd, &trigger, 2);

    // Reset our persistent buffer for the new incoming reading
    ctx->buf_pos = 0; 

    // 2. Loop until we receive the complete response
    while (1) {
        char temp_buf[64];
        int n = read(ctx->fd, temp_buf, sizeof(temp_buf));

        if (n > 0) {
            for (int i = 0; i < n; i++) {
                char c = temp_buf[i];

                // At the end of a line
                if (c == '\n' || c == '\r') {
                    if (ctx->buf_pos > 0) {
                        ctx->buffer[ctx->buf_pos] = '\0'; // Null terminate
                        *value_out = atoi(ctx->buffer);   // Convert to int

                        // Validate bounds
                        if (*value_out > max_val || *value_out < min_val) {
                            return 0; // Bad read
                        }
                        
                        return 1; // Success!
                    }
                }
                // Otherwise, keep building the string
                else if (ctx->buf_pos < sizeof(ctx->buffer) - 1) {
                    ctx->buffer[ctx->buf_pos++] = c;
                }
            }
        } else {
            usleep(1000); 
        }
    }
}

void serial_close(SerialContext *ctx)
{
	close(ctx->fd);
}