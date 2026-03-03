#define MIN_SENSOR_READ 0
#define MAX_SENSOR_READ 1024

// Context to hold the state of the serial connection
typedef struct {
    int fd;             // File descriptor
    char buffer[256];   // Buffer to hold partial data
    int buf_pos;        // Current position in buffer
} SerialContext;

int serial_init(SerialContext* ctx, const char* port_name);
int serial_read_int(SerialContext* ctx, int* value_out, int max_value, int min_value);
void serial_close(SerialContext* ctx);