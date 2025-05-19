#define _GNU_SOURCE

#include "../CHeaders/signalHandler.h"
#include "../CHeaders/utilities.h"
#include "../CHeaders/xerrori.h"

#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @brief Crea ed inizializza il thread gestore dei segnali.
 * @param mutex Mutex da passare al thread gestore dei segnali.
 * @param finishedGraph Puntatore alla flag che dice se la costruzione del grafo è finita.
 * @param mustShutdown Puntatore alla flag che dice se il programma deve terminare.
 */
void signalHandlerThreadInit(volatile bool* finishedGraph, volatile bool* mustShutdown) {
    pthread_t thread;
    signalHandlerData* data = malloc(sizeof(signalHandlerData));
    if (data == NULL) xtermina(LINEFILE, "malloc per struct del thread gestore dei segnali fallita");

    data -> finishedGraph = finishedGraph;
    data -> mustShutdown = mustShutdown;

    xpthread_create(&thread, NULL, &signalHandlerBody, data, LINEFILE);
    if (pthread_detach(thread) != 0) xtermina(LINEFILE, "pthread_detach del thread gestore dei segnali fallita");
}


/**
 * @brief Funzione eseguita dal thread gestore dei segnali, aspetta SIGINT e termina il programma.
 * @param arg Struct passata da signalHandlerThreadInit().
 */
void* signalHandlerBody(void* arg) {
    printf("Thread gestore dei segnali partito.\nPID del processo: %ld\n", (long) getpid());
    signalHandlerData* data = (signalHandlerData*) arg;

    sigset_t mask;
    if (sigemptyset(&mask) != 0) xtermina(LINEFILE, "sigemptyset nel thread gestore dei segnali fallita");
    if (sigaddset(&mask, SIGINT) != 0) xtermina(LINEFILE, "sigaddset nel thread gestore dei segnali fallita");

    int sig, result;

    while (true) {
        if (sigwait(&mask, &sig) != 0) xtermina(LINEFILE, "sigwait fallita nel thread gestore dei segnali");

        if (sig == SIGINT) {
            if (*(data -> finishedGraph) == false) printf("Costruzione del grafo in corso\n");
            else {
                *(data -> mustShutdown) = true;
                break;
            }
        }
    }

    free(data);

    pthread_exit(NULL);
}