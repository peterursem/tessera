#ifndef TESSERA
#define TESSERA

typedef struct {
    int active;
    int resolution;
    int framerate;
    int batch_size;
    int progress;
    char sensor_port[32];
} TesseraStatus;

volatile extern TesseraStatus app_status;

#endif