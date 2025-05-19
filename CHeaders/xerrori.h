#ifndef XERRORI_H
#define XERRORI_H

#define _GNU_SOURCE   // permette di usare estensioni GNU
#include <stdio.h>    // permette di usare scanf printf etc ...
#include <stdlib.h>   // conversioni stringa exit() etc ...
#include <stdbool.h>  // gestisce tipo bool
#include <assert.h>   // permette di usare la funzione ass
#include <string.h>   // funzioni per stringhe
#include <errno.h>    // richiesto per usare errno
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <pthread.h>
#include <stdarg.h> // Usato per i parametri multipli in xtermina()

#define LINEFILE __LINE__,__FILE__

// termina programma
void termina(const char*); 

// Dice al compilatore che xtermina() si comporta come printf a partire
// dal terzo parametro, e i parametri variadici iniziano dal quarto.
// Permette al compilatore di eseguire check sui parametri e il formato della stringa,
// come farebbe per la funzione printf().
void xtermina(int, char*, const char*, ...) __attribute__ ((format (printf, 3, 4)));

// operazioni su FILE *
FILE *xfopen(const char*, const char*, int, char*);

// operazioni su file descriptors
void xclose(int, int, char*);

// thread
void xperror(int, char*);

int xpthread_create(pthread_t*, const pthread_attr_t*,
                          void* (*start_routine) (void*), void*, int, char*);
int xpthread_join(pthread_t, void**, int, char*);

// mutex 
int xpthread_mutex_init(pthread_mutex_t* restrict mutex, const pthread_mutexattr_t* restrict attr, int, char*);
int xpthread_mutex_destroy(pthread_mutex_t*, int, char*);
int xpthread_mutex_lock(pthread_mutex_t*, int, char*);
int xpthread_mutex_unlock(pthread_mutex_t*, int, char*);

// semafori named e unnamed POSIX
sem_t *xsem_open(const char*, int, mode_t, unsigned int value, int, char*);
int xsem_unlink(const char*, int, char*);
int xsem_close(sem_t*, int, char*);
int xsem_init(sem_t*, int, unsigned int value, int, char*);
int xsem_destroy(sem_t*, int, char*);
int xsem_post(sem_t*, int, char*);
int xsem_wait(sem_t*, int, char*);


// condition variables
int xpthread_cond_init(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr, int, char*);
int xpthread_cond_destroy(pthread_cond_t*, int, char*);
int xpthread_cond_wait(pthread_cond_t* restrict cond, pthread_mutex_t* restrict mutex, int, char*);
int xpthread_cond_signal(pthread_cond_t*, int, char*);
int xpthread_cond_broadcast(pthread_cond_t*, int, char*);

#endif