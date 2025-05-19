#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    volatile bool* finishedGraph;
    volatile bool* mustShutdown;
} signalHandlerData;

void signalHandlerThreadInit(volatile bool*, volatile bool*);
void* signalHandlerBody(void*);

#endif