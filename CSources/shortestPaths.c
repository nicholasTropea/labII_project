#define _GNU_SOURCE

#include "../CHeaders/shortestPaths.h"
#include "../CHeaders/utilities.h"
#include "../CHeaders/dataStructures.h"
#include "../CHeaders/actors.h"
#include "../CHeaders/xerrori.h"

#include <fcntl.h> // Per O_RDONLY
#include <inttypes.h> // Per PRId32
#include <sys/select.h> // Per select()
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/times.h>

/**
 * @brief Legge i messaggi dalla pipe e crea i thread per il calcolo dei cammini minimi.
 * @param attori Array dei nodi attore creato in createActors().
 * @param mustShutdown Booleano per gestire l'arrivo di SIGINT.
 */
void pipeReader(size_t n, attore** attori, volatile bool* mustShutdown) {    
    // Apre la pipe in lettura
    int fd;

    /*
        Il ciclo è usato per lo stesso motivo della select() durante la lettura,
        se arrivasse SIGINT prima che un lettore apra la pipe, non verrebbe gestita
        correttamente dato che il programma sarebbe in attesa passiva.
    */

    while (true) {
        if (*mustShutdown) {
            fprintf(stderr, "Inizio attesa di 20 secondi.\n");
            sleep(20);
            fprintf(stderr, "Fine attesa di 20 secondi.\n");
            return; // Esce direttamente, non deve deallocare o chiudere file
        }

        fd = open("cammini.pipe", O_RDONLY | O_NONBLOCK);
        if (fd >= 0) break; // Pipe aperta da uno scrittore
        else if (errno == ENXIO) { // Pipe ancora non aperta in scrittura
            sleep(1);
            continue;
        }
        else xtermina(LINEFILE, "Apertura della pipe fallita");
    }

    message msg;
    ssize_t readVal;

    fprintf(stderr, "Inizio lettura dalla pipe.\n");

    // Legge dalla pipe
    while (!(*mustShutdown)) {
        /*
            Usato select + timeout dato che non è possibile rendere la pipe non bloccante
            (perché non posso modificare cammini.py per aggiungere i dovuti check sulla write),
            è necessario perché in un eventuale caso in cui il programma resta in attesa
            sulla pipe, potrebbe non notare il cambiamento di mustShutdown (arrivo di SIGINT)
            e quindi non funzionare come descritto dal professore.

            select() è una chiamata di sistema che permette ad un processo di, in questo caso,
            attendere che il file descriptor della pipe sia pronto per la lettura, evitando
            blocchi di attesa indefinita.
            
            Parametri:
                nfds: Valore più alto dei file descriptors monitorati + 1 (fd + 1 in questo caso)
                readfds: puntatore ad un fd_set con i file descriptors da monitorare in lettura (readfds)
                writefds: analogo di readfds ma per scrittura (NULL)
                exceptfds: analogo di readfds ma per errori eccezionali (NULL)
                timeout: tempo speso a monitorare i file descriptor prima di uscire (500ms)

            Funzioni:
                FD_ZERO: Inizializza un fd_set vuoto
                FD_SET: Aggiunge un file descriptor ad un fd_set
        */

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 500ms

        int retval = select(fd + 1, &readfds, NULL, NULL, &timeout);

        if (retval == -1) xtermina(LINEFILE, "select fallita durante check sulla pipe"); 
        else if (retval == 0) continue; // Timeout terminato
        else { // Pipe pronta in lettura
            readVal = read(fd, &msg, sizeof(msg));

            if (readVal < 0) xtermina(LINEFILE, "Lettura dalla pipe fallita");
            else if (readVal == 0) break;
            else if (readVal < sizeof(msg)) xtermina(LINEFILE, "Letto un messaggio incompleto dalla pipe");

            fprintf(stderr, "Creazione thread per codici: %" PRId32 " e %" PRId32 ".\n", msg.a, msg.b);
            createShortestPathThread(msg.a, msg.b, attori, n);
        }
    }

    fprintf(stderr, "Inizio attesa di 20 secondi.\n");
    sleep(20);
    fprintf(stderr, "Fine attesa di 20 secondi.\n");

    close(fd);
}

/**
 * @brief Crea un thread detached calcolatore di cammini minimi. 
 * @param a Intero a 32 bit rappresentante il codice del primo attore.
 * @param b Intero a 32 bit rappresentante il codice del secondo attore.
 * @param attori Array dei nodi attore. (Grafo)
 */
void createShortestPathThread(int32_t a, int32_t b, attore** attori, size_t n) {
    pthread_t thread;
    pathThreadData* data = malloc(sizeof(pathThreadData));
    if (data == NULL) xtermina(LINEFILE, "Allocazione della struct per thread calcolatore di cammini minimi fallita");

    data -> a = a;
    data -> b = b;
    data -> actors = attori;
    data -> size = n;

    xpthread_create(&thread, NULL, &pathThreadBody, data, LINEFILE);
    if (pthread_detach(thread) != 0) xtermina(LINEFILE, "pthread_detach del thread calcolatore di cammini fallita");
}

/**
 * @brief Funzione eseguita dal thread calcolatore di cammini.
 * @param arg Struttura passata da createShortestPathThread().
 */
void* pathThreadBody(void* arg) {
    clock_t timeStart = times(NULL);
    pathThreadData* data = (pathThreadData*) arg;

    fprintf(stderr, "Inizio thread per %" PRId32 " e %" PRId32 ".\n", data -> a, data -> b);

    // Crea il file
    char filename[50]; // Abbondante per evitare overflow
    sprintf(filename, "%" PRId32 ".%" PRId32, data -> a, data -> b);

    FILE* file = xfopen(filename, "w", LINEFILE);

    // Ricerca binaria per trovare a
    attore* actorA = *(attore**) bsearch(&(data -> a), data -> actors, data -> size, sizeof(attore*), &compareAttore);

    if (actorA == NULL) {
        fprintf(file, "Codice %" PRId32 " non valido\n", data -> a);
        fclose(file);
        printf("%" PRId32 ".%" PRId32 ": Codici invalidi. Tempo di elaborazione %.3f secondi", data -> a, data -> b, (double)(times(NULL) - timeStart) / sysconf(_SC_CLK_TCK));
        free(data);
        pthread_exit(NULL);
    }

    if (data -> a == data -> b) {
        char* actorString = actorToString(actorA);

        fprintf(file, "%s\n", actorString);

        free(actorString);
        fclose(file);
        printf("%" PRId32 ".%" PRId32 ": Lunghezza minima 0. Tempo di elaborazione %.3f secondi", data -> a, data -> b, (double)(times(NULL) - timeStart) / sysconf(_SC_CLK_TCK));
        pthread_exit(NULL);
    }

    // Ricerca binaria per trovare b
    attore* actorB = *(attore**) bsearch(&(data -> b), data -> actors, data -> size, sizeof(attore*), &compareAttore);

    if (actorB == NULL) {
        fprintf(file, "Codice %" PRId32 " non valido\n", data -> b);
        fclose(file);
        printf("%" PRId32 ".%" PRId32 ": Codici invalidi. Tempo di elaborazione %.3f secondi", data -> a, data -> b, (double)(times(NULL) - timeStart) / sysconf(_SC_CLK_TCK));
        free(data);
        pthread_exit(NULL);
    }

    // Crea abr e aggiunge a
    abr* explored = malloc(sizeof(abr));
    if (explored == NULL) xtermina(LINEFILE, "Allocazione dell'abr fallita");

    explored -> shuffledCode = shuffle(actorA -> codice);
    explored -> left = NULL;
    explored -> right = NULL;

    // Crea coda di ricerca e aggiunge a senza genitore
    circularQueue* queue = queueCreate();
    enqueue(queue, actorA -> codice);

    // Crea array usato per open addressing sui genitori nella ricerca del cammino minimo
    int* parents = calloc((data -> actors)[data -> size - 1] -> codice, sizeof(int));
    if (parents == NULL) xtermina(LINEFILE, "Allocazione array dei genitori per open addressing fallita");

    // Aggiunge -1 come padre di a
    parents[actorA -> codice] = -1;
    
    bool found = false;
    int currentCode;

    // BFS
    while (!queueIsEmpty(queue)) {
        currentCode = dequeue(queue);
        attore* currentActor = *(attore**) bsearch(&currentCode, data -> actors, data -> size, sizeof(attore*), &compareAttore);

        // Scorre i coprotagonisti
        for (size_t i = 0; i < currentActor -> numcop; i++) {
            int coprotCode = (currentActor -> cop)[i];
            
            // Se trova B, setta il genitore ed esce
            if (coprotCode == actorB -> codice) {
                parents[coprotCode] = currentCode;
                found = true;
                break;
            }

            // Se attore già esplorato salta
            if (abrContains(explored, shuffle(coprotCode))) continue;

            // Se attore non esplorato lo aggiunge all'ABR, alla coda e setta il parent
            abrAdd(explored, shuffle(coprotCode));
            enqueue(queue, coprotCode);
            parents[coprotCode] = currentCode;

        }

        if (found) break;
    }

    if (!found) {
        fprintf(file, "Non esistono cammini da %d a %d\n", actorA -> codice, actorB -> codice);
        printf("%" PRId32 ".%" PRId32 ": Lunghezza minima 0. Tempo di elaborazione %.3f secondi", data -> a, data -> b, (double)(times(NULL) - timeStart) / sysconf(_SC_CLK_TCK));
        fclose(file);
        abrFree(explored);
        free(data);
        freeQueue(queue);
        pthread_exit(NULL);
    }

    clock_t timeEnd = times(NULL);
    double elapsed_time = (double)(timeEnd - timeStart) / sysconf(_SC_CLK_TCK);

    fprintf(stderr, "Inizio scrittura su %" PRId32 ".%" PRId32 ".\n", data -> a, data -> b);
    size_t len = printShortestPath(actorB, file, parents, data -> actors, data -> size);
    fprintf(stderr, "Termine scrittura su %" PRId32 ".%" PRId32 ".\n", data -> a, data -> b);

    printf("%" PRId32 ".%" PRId32 ": Lunghezza minima %ld. Tempo di elaborazione %.3f secondi.\n", data -> a, data -> b, len, elapsed_time);

    fprintf(stderr, "Termine thread per %" PRId32 " e %" PRId32 ".\n", data -> a, data -> b);

    // Clean-up
    fclose(file);
    freeQueue(queue);
    abrFree(explored);
    free(data);
    free(parents);

    pthread_exit(NULL);
}

/**
 * @brief Stampa sul file gli attori appartenenti al cammino minimo da start a target.
 * @param target Attore target del cammino.
 * @param tree ABR costruito dal thread calcolatore di cammini minimi.
 * @param file File su cui scrivere il cammino.
 * @return Lunghezza del cammino calcolato.
 */
size_t printShortestPath(attore* target, FILE* file, int* parents, attore** attori, size_t n) {
    attore* currentActor = target;
    int currentCode = currentActor -> codice;
    stack* stack = stackCreate();

    // Popola lo stack
    while (true) {
        stackPush(stack, currentActor);
        if (parents[currentCode] == -1) break;

        currentActor = *(attore**) bsearch(&parents[currentCode], attori, n, sizeof(attore*), &compareAttore);
        currentCode = currentActor -> codice;
    }

    size_t len = 0; // Lunghezza del cammino
    char* actorString;

    // Stampa in ordine
    while (!stackIsEmpty(stack)) {
        len++;
        target = stackPop(stack);
        actorString = actorToString(target);
        fprintf(file, "%s\n", actorString);
        free(actorString);
    }

    // Clean-up
    stackFree(stack);

    return len - 1;
}
