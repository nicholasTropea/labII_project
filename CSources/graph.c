#define _GNU_SOURCE

#include "../CHeaders/graph.h"
#include "../CHeaders/utilities.h"
#include "../CHeaders/actors.h"
#include "../CHeaders/xerrori.h"


#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 20

/**
 * @brief Esegue il parsing di una linea nel buffer e aggiorna il nodo dell'attore
 * @param line Linea da parsare in formato: actorCode\t#coprotagonisti\tcoprot1Code\tcoprot2Code\t...\tcoprotNCode\t\n
 * @param arr Array dei nodi attore creato in createActors().
 * @param n Size dell'array arr
 */
void updateCoprotagonists(char* line, attore** arr, size_t n) {
    char* token;
    char* savePtr; // Usato per strtok_r
    int index = 0;

    // Parsa il codice
    token = strtok_r(line, "\t", &savePtr);
    if (token == NULL) {
        freeAttori(arr, n);
        xtermina(LINEFILE, "strtok fallita nel thread consumatore: %ld", (long) gettid());
    }
    int code = atoi(token);

    // Ricerca binaria del nodo corretto
    attore* actor = *(attore**) bsearch(&code, arr, n, sizeof(attore*), &compareAttore);

    actor -> codice = code;

    // Parsa il numero di coprotagonisti
    token = strtok_r(NULL, "\t", &savePtr);
    if (token == NULL) {
        freeAttori(arr, n);
        xtermina(LINEFILE, "strtok fallita nel thread consumatore: %ld", (long) gettid());
    }
    actor -> numcop = atoi(token);

    // Alloca array dei coprotagonisti e aggiungilo al nodo
    int* temp = malloc((actor -> numcop) * sizeof(int));
    if (temp == NULL) xtermina(LINEFILE, "Allocazione dell'array dei coprotagonisti fallita nel thread consumatore: %ld", (long) gettid());
    actor -> cop = temp;

    // Parsa i codici dei coprotagonisti
    while ((token = strtok_r(NULL, "\t", &savePtr)) != NULL && index < actor -> numcop) {
        (actor -> cop)[index++] = atoi(token); 
    }

    // Check correttezza del file grafo.txt
    if (index != actor -> numcop) {
        freeAttori(arr, n);
        xtermina(LINEFILE, "Mismatch nel numero dei coprotagonisti dato e quello effettivo dell'attore: %d\n\tTrovati: %d\n\tPrevisti: %d", code, index, actor -> numcop);
    }
}


/**
 * @brief Funzione eseguita dai thread consumatori.
 * @arg Struct con i dati passata dal thread creatore.
 */
void* workerBody(void* arg) {
    workerData* data = (workerData*) arg;

    char* current; // Linea attuale

    do {
        // Zona critica
        xsem_wait(data -> itemsIn, LINEFILE);
        xpthread_mutex_lock(data -> mutex, LINEFILE);

        current = data -> buffer[*(data -> index) % BUFFER_SIZE];
        *(data -> index) += 1;

        xpthread_mutex_unlock(data -> mutex, LINEFILE);
        xsem_post(data -> freeSlots, LINEFILE);
        // Fine zona critica

        if (!current) continue;

        // Update dei coprotagonisti dell'attore
        updateCoprotagonists(current, data -> attori, data -> attoriSize);

        free(current); // Le linee erano state duplicate dal produttore (copy)
    } while (current != NULL);

    pthread_exit(NULL);
}


/**
 * @brief Funzione eseguita dal thread produttore.
 * @param filePath Percorso del file grafo.txt
 * @param buffer Buffero produttore/consumatori.
 * @param freeSlots Semaforo degli elementi liberi.
 * @param itemsIn Semaforo degli elementi occupati.
 * @param n Numero dei threads consumatori.
 */
void producerBody(FILE* file, char** buffer, sem_t* freeSlots, sem_t* itemsIn, size_t n) {
    char* line = NULL; // Buffer per la linea
    size_t len = 0; // Dimensione iniziale del buffer
    ssize_t read; // Numero di caratteri letti (-1 per fine del file o errore)

    int index = 0;

    // Ciclo di lettura dal file grafo.txt
    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1 || line[0] == '\n') continue; // Salta ultima linea o linee vuote

        char* copy = strdup(line);
        if (copy == NULL) xtermina(LINEFILE, "strdup fallita nel thread produttore");

        // Aggiunge la riga letta al buffer
        xsem_wait(freeSlots, LINEFILE);
        buffer[index++ % BUFFER_SIZE] = copy;
        xsem_post(itemsIn, LINEFILE);
    }

    // Check se c'è stato un errore durante la lettura dal file
    if (ferror(file)) xtermina(LINEFILE, "Lettura dal file grafo.txt fallita");

    free(line); // copy verrà deallocata dai thread consumatori

    // Aggiunge segnale di termine ai thread consumatori
    for (size_t i = 0; i < n; i++) {
        xsem_wait(freeSlots, LINEFILE);
        buffer[index++ % BUFFER_SIZE] = NULL;
        xsem_post(itemsIn, LINEFILE);
    }
}


/**
 * @brief Processa il file grafo.txt riempiendo i campi cop e numcop dei nodi attore utilizzando uno schema produttore/consumatori
 * @param filePath Percorso del file grafo.txt
 * @param n Argomento numconsumatori passato da linea di comando e convertito ad intero.
 * @param attori Array dei nodi attore creato in createActors().
 */
void processGraph(char* filePath, size_t n, attore** attori, size_t attoriSize) {
    FILE* file = xfopen(filePath, "r", LINEFILE);

    // Crea il buffer
    char** buffer = malloc(BUFFER_SIZE * sizeof(char*));
    if (buffer == NULL) handleWithFileError("Allocazione del buffer produttore/consumatori fallita", file);

    // Crea i thread
    pthread_t* threads = malloc(n * sizeof(pthread_t));
    workerData* threadData = malloc(n * sizeof(workerData));
    if (threads == NULL || threadData == NULL) handleWithFileError("Allocazione degli array dei threads fallita", file);

    sem_t freeSlots, itemsIn;
    xsem_init(&freeSlots, 0, BUFFER_SIZE, LINEFILE);
    xsem_init(&itemsIn, 0, 0, LINEFILE);

    pthread_mutex_t mutex;
    xpthread_mutex_init(&mutex, NULL, LINEFILE);

    int cindex = 0;

    int error; // Variabile di check per errore

    // Crea i thread consumatori
    for (size_t i = 0; i < n; i++) {
        // Riempio i dati da passare al thread
        threadData[i].buffer = buffer;
        threadData[i].index = &cindex;
        threadData[i].itemsIn = &itemsIn;
        threadData[i].freeSlots = &freeSlots;
        threadData[i].mutex = &mutex;
        threadData[i].attori = attori;
        threadData[i].attoriSize = attoriSize;

        // Fa partire il thread
        xpthread_create(&threads[i], NULL, &workerBody, &threadData[i], LINEFILE);
    }

    // Legge grafo.txt ed inserisce nel buffer, manda il segnale di termine ai consumatori
    producerBody(file, buffer, &freeSlots, &itemsIn, n);
    fclose(file);

    // Aspetta che i consumatori terminino
    for (size_t i = 0; i < n; i++) xpthread_join(threads[i], NULL, LINEFILE);

    // Cleanup
    xsem_destroy(&freeSlots, LINEFILE);
    xsem_destroy(&itemsIn, LINEFILE);
    xpthread_mutex_destroy(&mutex, LINEFILE);
    free(buffer);
    free(threads);
    free(threadData);
}
