#ifndef PATHS_H
#define PATHS_H

#include "actors.h"
#include "dataStructures.h"

#include <stdint.h> // Per usare int32_t, probabilmente non necessario ma per sicurezza
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    int32_t a;
    int32_t b;
} message;

typedef struct {
    int32_t a; // Codice dell'attore iniziale
    int32_t b; // Codice dell'attore destinazione
    attore** actors; // Array degli attori
    size_t size; // Size dell'array degli attori
} pathThreadData;

void pipeReader(size_t, attore**, volatile bool*);
void createShortestPathThread(int32_t, int32_t, attore**, size_t);
void* pathThreadBody(void*);
size_t printShortestPath(attore*, FILE*, int*, attore**, size_t);

#endif