#define _GNU_SOURCE

#include "../CHeaders/utilities.h"
#include "../CHeaders/xerrori.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h> // Usato per isdigit()

/**
 * @brief Stampa messaggio di errore durante la creazione dell'array attori e termina il programma.
 * @param msg Messaggio d'errore da stampare su stderr.
 * @param file File da chiudere.
 */
void handleWithFileError(char* msg, FILE* file) {
    fclose(file);
    xtermina(LINEFILE, "%s", msg);
}


/**
 * @brief Funzione di controllo degli argomenti passati da linea di comando.
 * @param argc Numero di argomenti passati.
 * @param argv Array degli argomenti passati.
 * @return true se gli argomenti passati sono validi, false altrimenti.
 */
bool validateArguments(int argc, char* argv[]) {
    // Controllo sul numero di parametri
    if (argc != 4) return false;

    // ============================= Controllo numconsumatori =============================
    char* n = argv[3];

    // Check per empty string
    if (n == NULL || *n == '\0') return false;

    // Salto il segno
    if (*n == '+') n++;

    // Se non ci sono cifre dopo il segno non è valido
    if (*n == '\0') return false;

    // Controllo i caratteri rimanenti
    while (*n) {
        if (!isdigit(*n)) return false;
        n++;
    }

    return true;
}
