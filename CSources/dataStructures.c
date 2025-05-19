#define _GNU_SOURCE

#include "../CHeaders/dataStructures.h"
#include "../CHeaders/utilities.h"
#include "../CHeaders/xerrori.h"

#include <stdlib.h>

// =============================== ABR =============================== //

/**
 * @brief Controlla se un codice è presente nell'ABR.
 * @param root Radice dell'ABR.
 * @param code Codice da cercare.
 * @return true se il codice è presente nell'ABR, false altrimenti.
 */
bool abrContains(abr* root, int code) {
    if (!root) xtermina(LINEFILE, "abrContains eseguita su root invalida");
    
    abr* current = root;

    while (current != NULL) {
        if (code == current -> shuffledCode) return true;
        
        if (code < current -> shuffledCode)
            current = current -> left;
        else
            current = current -> right;
    }
    
    return false;
}

/**
 * @brief Aggiunge un codice all'ABR.
 * @param root Radice dell'ABR.
 * @param code Codice da aggiungere.
 */
void abrAdd(abr* root, int code) {
    if (!root) xtermina(LINEFILE, "abrAdd() eseguita su root invalida");
    
    abr* current = root;
    abr** target = NULL;

    while (true) {
        if (code >= current -> shuffledCode) {
            if (current -> right == NULL) {
                target = &(current -> right);
                break;
            }

            current = current -> right;
        } else {
            if (current -> left == NULL) {
                target = &(current -> left);
                break;
            }

            current = current -> left;
        }
    }
    
    // Create new node
    abr* node = malloc(sizeof(abr));
    if (node == NULL) xtermina(LINEFILE, "Allocazione del nodo abr fallita");
    
    node -> shuffledCode = code;
    node -> left = NULL;
    node -> right = NULL;
    
    *target = node;
}

/**
 * @brief Dealloca la memoria occupata dall'ABR.
 * @param root Radice dell'ABR.
 */
void abrFree(abr* root) {
    abrFreeHelper(root);
    free(root);
}

/**
 * @brief Helper function di abrFree().
 * @param node Nodo ABR.
 */
void abrFreeHelper(abr* node) {
    if (!node) return;

    if (node -> left) abrFreeHelper(node -> left);
    if (node -> right) abrFreeHelper(node -> right);

    free(node -> left);
    free(node -> right);
}

/**
 * @brief Mescola i bit del codice passato.
 * @param n Codice da mescolare.
 * @return Codice mescolato.
 */
int shuffle(int n) {
    return ((((n & 0x3F) << 26) | ((n >> 6) & 0x3FFFFFF)) ^ 0x55555555);
}

/**
 * @brief Dato un codice mescolato con shuffle() calcola il codice originale.
 * @param n Codice mescolato.
 * @return Codice originale
 */
int unshuffle(int n) {
    return ((((n >> 26) & 0x3F) | ((n & 0x3FFFFFF) << 6)) ^ 0x55555555);
}

// =============================== CIRCULAR QUEUE =============================== //

/**
 * @brief Crea ed inizializza una nuova coda fifo circolare.
 * @return Puntatore alla coda creata.
 */
circularQueue* queueCreate() {
    circularQueue* queue = malloc(sizeof(circularQueue));
    if (queue == NULL) xtermina(LINEFILE, "Allocazione della coda fallita");

    // Inizializza la nuova coda
    int* arr = malloc(INITIAL_QUEUE_SIZE * sizeof(int));
    if (arr == NULL) xtermina(LINEFILE, "Allocazione dell'array della coda fallita");

    queue -> head = 0;
    queue -> tail = -1;
    queue -> size = 0;
    queue -> capacity = INITIAL_QUEUE_SIZE;
    queue -> items = arr;

    return queue;
}

/**
 * @brief Verifica se la coda è piena.
 * @param queue Puntatore alla coda.
 * @return true se la coda è piena, false altrimenti.
 */
bool queueIsFull(circularQueue* queue) {
    if (!queue) xtermina(LINEFILE, "queueIsFull() eseguito su coda invalida");
    return queue -> size == queue -> capacity;
}

/**
 * @brief Verifica se la coda è vuota.
 * @param queue Puntatore alla coda.
 * @return true se la coda è vuota, false altrimenti.
 */
bool queueIsEmpty(circularQueue* queue) {
    if (!queue) xtermina(LINEFILE, "queueIsEmpty() eseguita su coda invalida");
    return queue -> size == 0;
}

/**
 * @brief Ridimensiona la coda.
 * @param queue Puntatore alla coda.
 */
void resizeQueue(circularQueue* queue) {
    int newCapacity = queue -> capacity * 2;
    int* newItems = realloc(queue -> items, newCapacity * sizeof(int));
    
    if (!newItems) xtermina(LINEFILE, "Riallocazione della coda fallita");
    
    // Se la coda è spezzata (tail < head), sistema gli elementi
    if (queue -> tail < queue -> head) {
        // Copia gli elementi dalla posizione 0 a tail nella nuova area
        for (size_t i = 0; i <= queue -> tail; i++) {
            newItems[queue -> capacity + i] = newItems[i];
        }

        queue -> tail += queue -> capacity;
    }
    
    queue -> items = newItems;
    queue -> capacity = newCapacity;
    
    fprintf(stderr, "Coda ridimensionata a capacità %d\n", newCapacity);
}

/**
 * @brief Aggiunge un codice infondo alla coda.
 * @param queue Puntatore alla coda.
 * @param code Codice da aggiungere.
 */
void enqueue(circularQueue* queue, int code) {
    if (queueIsFull(queue)) resizeQueue(queue);
    
    queue -> tail = (queue -> tail + 1) % queue -> capacity;
    queue -> items[queue -> tail] = code;
    queue -> size++;
}

/**
 * @brief Rimuove e restituisce la testa della coda.
 * @param queue Puntatore alla coda.
 * @return Codice in testa alla coda.
 */
int dequeue(circularQueue* queue) {
    if (!queue) xtermina(LINEFILE, "dequeue() eseguito su coda invalida");
    if (queueIsEmpty(queue)) xtermina(LINEFILE, "dequeue() eseguito su coda vuota");
    
    int code = queue -> items[queue -> head];
    queue -> head = (queue -> head + 1) % queue -> capacity;
    queue -> size--;
    return code;
}

/**
 * @brief Libera la memoria occupata da una coda.
 * @param queue Puntatore alla coda.
 */
void freeQueue(circularQueue* queue) {
    free(queue -> items);
    free(queue);
}

// =============================== STACK =============================== //

/**
 * @brief Crea ed inizializza uno stack vuoto.
 * @return Puntatore allo stack creato.
 */
stack* stackCreate() {
    stack* s = malloc(sizeof(stack));
    if (s == NULL) xtermina(LINEFILE, "Allocazione dello stack fallita");

    s -> top = NULL;

    return s;
}

/**
 * @brief Verifica se lo stack passato è vuoto.
 * @param s stack.
 * @return true se lo stack passato è vuoto, false altrimenti.
 */
bool stackIsEmpty(stack* s) {
    return s -> top == NULL;
}

/**
 * @brief Aggiunge un attore allo stack.
 * @param s Puntatore ad uno stack.
 * @param a Puntatore ad un attore.
 */
void stackPush(stack* s, attore* a) {
    stack_node* node = malloc(sizeof(stack_node));
    if (node == NULL) xtermina(LINEFILE, "Allocazione di un nodo dello stack fallita");

    node -> actor = a;
    
    if (stackIsEmpty(s)) node -> previous = NULL;
    else node -> previous = s -> top;

    s -> top = node;
}

/**
 * @brief Restituisce e rimuove l'attore incima allo stack deallocando la memoria occupata dal suo nodo stack.
 * @param s Puntatore ad uno stack.
 * @return Puntatore all'attore rimosso se lo stack non è vuoto, NULL altrimenti.
 */
attore* stackPop(stack* s) {
    if (!s) xtermina(LINEFILE, "stackPop() eseguita su stack invalido");

    if (stackIsEmpty(s)) return NULL;
    
    stack_node* temp = s -> top;
    
    attore* result = temp -> actor;
    s -> top = temp -> previous;
    free(temp);
    
    return result;
}

/**
 * @brief Dealloca la memoria dello stack e dei suoi nodi.
 * @param s Puntatore ad uno stack.
 */
void stackFree(stack* s) {
    stack_node* temp;
    
    while (s -> top != NULL) {
        temp = s -> top -> previous;
        free(s -> top);
        s -> top = temp;
    }

    free(s);
}