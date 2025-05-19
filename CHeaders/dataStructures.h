#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "actors.h"
#include <stdbool.h>

// =============================== ABR =============================== //

typedef struct abr_node {
    int shuffledCode;
    struct abr_node* left;
    struct abr_node* right;
} abr;

bool abrContains(abr*, int);
void abrAdd(abr*, int);
void abrFree(abr*);
void abrFreeHelper(abr*);
int shuffle(int);
int unshuffle(int);

// =============================== CIRCULAR QUEUE =============================== //

#define INITIAL_QUEUE_SIZE 250000

typedef struct {
    int head; // Indice del primo elemento
    int tail; // Indice dell'ultimo elemento
    size_t size; // Numero di elementi nella coda
    size_t capacity; // Capacità massima della coda
    int* items;
} circularQueue;

circularQueue* queueCreate();
bool queueIsFull(circularQueue*);
bool queueIsEmpty(circularQueue*);
void resizeQueue(circularQueue*);
void enqueue(circularQueue*, int);
int dequeue(circularQueue*);
void freeQueue(circularQueue*);

// =============================== STACK =============================== //

typedef struct stack_node {
    attore* actor;
    struct stack_node* previous;
} stack_node;

typedef struct {
    stack_node* top;
} stack;

stack* stackCreate();
bool stackIsEmpty(stack*);
void stackPush(stack*, attore*);
attore* stackPop(stack*);
void stackFree(stack*);

#endif