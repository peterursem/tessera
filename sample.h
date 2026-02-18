// Context to hold the state of the serial connection
typedef struct {
    int fd;             // File descriptor
    char buffer[256];   // Buffer to hold partial data
    int buf_pos;        // Current position in buffer
} SerialContext;

// Prototypes
int serial_init(SerialContext* ctx, const char* port_name, int baud_rate);
int serial_read_int(SerialContext* ctx, int* value_out);
void serial_close(SerialContext* ctx);