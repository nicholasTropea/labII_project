#define _GNU_SOURCE

#include "../CHeaders/actors.h"
#include "../CHeaders/utilities.h"
#include "../CHeaders/graph.h"
#include "../CHeaders/signalHandler.h"
#include "../CHeaders/shortestPaths.h"
#include "../CHeaders/xerrori.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LINEFILE __LINE__,__FILE__

/**
 * @file cammini.c
 * @brief Legge i dati del grafo degli autori ed effettua il calcolo di cammini minimi. 
 */

int main(int argc, char* argv[]) {
    // Convalida gli argomenti passati da linea di comando
    if (!validateArguments(argc, argv)) {
        printf("Errore: Utilizzo del programma invalido.\nUso: %s pathTo(nomi.txt) pathTo(grafo.txt) numConsumatori", argv[0]);
        exit(2);
    }

    // Blocca SIGINT
    sigset_t mask;
    if (sigemptyset(&mask) != 0) xtermina(LINEFILE, "sigemptyset() nel main fallita");
    if (sigaddset(&mask, SIGINT) != 0) xtermina(LINEFILE, "sigaddset() nel main fallita");
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) xtermina(LINEFILE, "pthread_sigmask() fallita nel main");

    // Crea thread gestore dei segnali (RUNNATO COME DETACHED)
    volatile bool finishedGraph = false; // false fino a che non elabora tutto il grafo
    volatile bool mustShutdown = false; // false fino a che non arriva SIGINT dopo il completamento dell'elaborazione del grafo
    signalHandlerThreadInit(&finishedGraph, &mustShutdown);

    // Lettura di nomi.txt e creazione dell'array dei nodi attore
    size_t attoriSize;
    attore** attori = createActors(argv[1], &attoriSize);

    // Lettura di grafo.txt e riempimento dei campi numcop e cop degli attori
    processGraph(argv[2], atoi(argv[3]), attori, attoriSize);

    // NOTA: Non mi serve un mutex dato che l'unica scrittura è questa, oltretutto atomica su architetture moderne
    finishedGraph = true; // Comunica al thread gestore dei segnali che ha finito
        
    // Crea la named pipe di comunicazione
    int e = mkfifo("cammini.pipe", 0660);
    if (e == 0) fprintf(stderr, "Named pipe creata.\n");
    else if (errno == EEXIST) fprintf(stderr, "Pipe già esistente.\n");
    else xtermina(LINEFILE, "Creazione della named pipe fallita");

    pipeReader(attoriSize, attori, &mustShutdown);

    // Elimina la named pipe creata
    if (unlink("cammini.pipe") == -1) xtermina(LINEFILE, "Errore nella distruzione della named pipe");

    freeAttori(attori, attoriSize);

    return 0;
}