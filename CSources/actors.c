#define _GNU_SOURCE

#include "../CHeaders/actors.h"
#include "../CHeaders/xerrori.h"
#include "../CHeaders/utilities.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_ACTORS_CAPACITY 383965

/**
 * @brief Dealloca la memoria dell'array degli attori.
 * @param arr Puntatore all'array.
 * @param size Lunghezza dell'array.
 */
void freeAttori(attore** arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (!arr[i]) continue;

        if (arr[i] -> nome) free(arr[i] -> nome);
        
        if (arr[i] -> cop) free(arr[i] -> cop);

        free(arr[i]);
    }

    free(arr);
}

/**
 * @brief Crea l'array dei nodi attore leggendo il file nomi.txt
 * @param filePath Percorso del file nomi.txt
 * @param arrSize Puntatore alla variabile contenente la size dell'array, che viene aggiornato.
 * @return Array degli attori.
 */
attore** createActors(char* filePath, size_t* arrSize) {
    FILE* file = xfopen(filePath, "r", LINEFILE);

    // Inizializza array dei nodi attore
    size_t size = INITIAL_ACTORS_CAPACITY;
    attore** attori = malloc(size * sizeof(attore*));
    if (attori == NULL) handleWithFileError("Allocazione dell'array degli attori fallita", file);

    char* line = NULL; // Buffer per la linea
    size_t len = 0; // Dimensione iniziale del buffer
    ssize_t read; // Numero di caratteri letti (-1 per fine del file o errore)
    size_t counter = 0; // Counter per l'array

    // Ciclo di lettura dal file nomi.txt
    while ((read = getline(&line, &len, file)) != -1) {
        if (read <= 1 || line[0] == '\n') continue; // Salta ultima linea o linee vuote
        
        // Resize dell'array degli attori
        if (counter >= size) {
            size *= 1.5;

            attore** tempAttori = realloc(attori, size * sizeof(attore*));
            if (tempAttori == NULL) {
                freeAttori(attori, size / 1.5);
                free(line);
                handleWithFileError("Riallocazione dell'array degli attori fallita", file);
            }

            attori = tempAttori;
        }

        // Creazione nodo attuale
        attore* current = malloc(sizeof(attore));
        if (current == NULL) {
            freeAttori(attori, size);
            free(line);
            handleWithFileError("Allocazione di un nodo attore fallita", file);
        }

        // Parsing della linea attuale
        char* token = strtok(line, "\t");
        if (token == NULL) {
            freeAttori(attori, size);
            free(current);
            free(line);
            handleWithFileError("Linea del file nomi.txt mal formattata", file);
        }
        current -> codice = atoi(token);

        token = strtok(NULL, "\t");
        if (token == NULL) {
            freeAttori(attori, size);
            free(current);
            free(line);
            handleWithFileError("Linea del file nomi.txt mal formattata", file);
        }
        current -> nome = strdup(token);
        if (current -> nome == NULL) {
            freeAttori(attori, size);
            free(current);
            free(line);
            handleWithFileError("strdup fallita durante la creazione di un nodo attore", file);
        }

        token = strtok(NULL, "\t");
        if (token == NULL) {
            freeAttori(attori, size);
            free(current -> nome);
            free(current);
            free(line);
            handleWithFileError("Linea del file nomi.txt mal formattata", file);
        }
        current -> anno = atoi(token);

        // Aggiunta del nodo attuale all'array
        attori[counter++] = current;
    }

    // Check se c'è stato un errore durante la lettura dal file
    if (ferror(file)) {
        perror("Errore: Lettura dal file nomi.txt fallita");
        freeAttori(attori, size);
        free(line);
        fclose(file);
        exit(2);
    }

    free(line);

    // Resize dell'array degli attori (non necessario dato che dovrebbero essere precisi, ma non si sa mai)
    if (counter < INITIAL_ACTORS_CAPACITY) {
        size = counter;

        attore** tempAttori = realloc(attori, size * sizeof(attore*));
        if (tempAttori == NULL) {
            freeAttori(attori, size);
            handleWithFileError("Riallocazione dell'array degli attori fallita", file);
        }

        attori = tempAttori;
    }

    fclose(file);
    *arrSize = size;
    return attori;
}

/**
 * @brief Compara due attori per codice.
 * @param key Nodo da cercare.
 * @param element Nodo con cui si sta confrontando.
 * @return Intero rappresentante il risultato della comparazione (< 0 -> oggetto cercato <, 0 -> oggetto cercato uguale, > 0 -> oggetto cercato maggiore)
 */
int  compareAttore(const void* key, const void* element) {
    int codice = *(const int*) key;
    const attore* a = *(const attore* const*) element;
    return codice - (a -> codice);
}

/**
 * @brief Restituisce le informazioni dell'attore formattate: codice\tnome\tannoDiNascita
 * @param a Attore da calcolare.
 * @return Stringa formattata allocata dinamicamente.
 */
char* actorToString(attore* a) {
    if (!a) xtermina(LINEFILE, "actorToString() chiamata su attore invalido");
    if (!a -> nome) xtermina(LINEFILE, "actorToString() chiamata su attore con nome invalido");

    char string[100]; // Abbastanza per evitare overflow
    
    sprintf(string, "%d\t%s\t%d\t", a -> codice, a -> nome, a -> anno);

    return strdup(string);
}