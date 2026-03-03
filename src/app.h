#ifndef TESSERA
#define TESSERA

typedef struct {
    int active;
    const int resolution;
    const int framerate;
    const int batch_size;
    int progress;
    const char sensor_port[32];
} TesseraStatus;

volatile TesseraStatus app_status;

#endif