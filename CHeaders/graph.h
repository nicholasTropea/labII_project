#ifndef GRAPH_H
#define GRAPH_H

#include "actors.h"
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct {
    char** buffer; // Buffer produttore/consumatori
    int* index; // Indice dei consumatori
    pthread_mutex_t* mutex; // Mutex dei consumatori
    sem_t* freeSlots; // Semaforo degli elementi liberi
    sem_t* itemsIn; // Semaforo degli elementi occupati
    attore** attori; // Array degli attori
    int attoriSize; // Size dell'array degli attori
} workerData;

void updateCoprotagonists(char*, attore**, size_t);
void* workerBody(void*);
void producerBody(FILE*, char**, sem_t*, sem_t*, size_t);
void processGraph(char*, size_t, attore**, size_t);

#endif