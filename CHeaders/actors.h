#ifndef ACTORS_H
#define ACTORS_H

#include <stddef.h>

typedef struct {
    int codice; // Codice dell'attore
    char* nome; // Nome dell'attore
    int anno; // Anno di nascita dell'attore
    int numcop; // Numero dei coprotagonisti dell'attore
    int* cop; // Array contenente i codici dei coprotagonisti dell'attore
} attore;

attore** createActors(char*, size_t*);
void freeAttori(attore**, size_t);
int compareAttore(const void*, const void*);
char* actorToString(attore*);
void printActors(attore**, size_t);

#endif