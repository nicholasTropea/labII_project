# Progetto per il corso di Laboratorio II, anno accademico 2024/2025 di Nicholas Riccardo Tropea.

## Struttura della directory  
Le subdirectory `CSources` e `javaSources` contengono rispettivamente i file sorgente `C` e `Java`, inoltre è presente una subdirectory `CHeaders` contenente i file `.h` rispettivi alle sorgenti C.

## Compilazione  
Per compilare il programma è sufficiente runnare `make`, questo genererà i file `.class` e l'eseguibile `cammini.out` nella directory principale, mentre creerà la subdirectory `CObjects` contenente i file `.o`.

## Parsing delle linee di name.basics.tsv in CreaGrafo.java  
Il parsing delle linee del file `name.basics.tsv` avviene nel metodo `parseTSV`, che tokenizza la linea separandola quando trova `\t` con i metodi `substring()` e `indexOf()`, salva i token in un array e li conta per evitare di restituire linee errate.

## Implementazione della coda FIFO  
La coda FIFO è implementata come un array dinamico circolare, questa struttura è stata scelta per l'efficienza in tempo `O(1)` delle operazioni da fare e per l'efficienza in memoria `O(n)`.  
L'implementazione delle funzioni della coda è presente nel file `dataStructures.c`, mentre la struttura si trova nel file `dataStructures.h` e contiene due indici `head` e `tail`, rispettivamente per gli elementi in testa e in coda, un campo `size` rappresentante il numero di elementi presenti nella coda, il campo `capacity` che rappresenta la capacità massima della coda e un array di interi `items`, i quali sono gli effettivi "nodi" nella coda.

## Ricostruzione dei nodi intermedi  
La ricostruzione dei nodi intermedi avviene attraverso l'array `parents`, usato per effettuare open addressing, dove se l'attore `i` ha codice `i.code`, in `parents[i.code]` si trova il codice del suo "genitore" in senso gerarchico nella ricerca.  
La ricostruzione del cammino quindi avviene semplicemente scorrendo l'array `parents` e mettendo gli elementi in uno stack, da cui verranno poi rimossi e scritti nel file.

## Funzionamento del thread gestore dei segnali  
La comunicazione tra il thread gestore dei segnali e il programma è molto semplice:  
il programma fa partire il thread gestore passandogli il puntatore a due variabili booleane `finishedGraph` e `mustShutdown`.  
Il thread quindi prosegue ad attendere il segnale `SIGINT` e quando esso arriva:
1. Se `finishedGraph` è `false`, stampa il messaggio di costruzione del grafo.
2. Se `finishedGraph` è `true`, setta `mustShutdown` a `true` per comunicare al programma che deve attendere 20 secondi e terminare.

Le variabili booleane sono rese `volatile` per motivi di ottimizzazione del compilatore, e non è stato usato un mutex dato che non ci sono race condition: `finishedGraph` verrà scritta solo una volta dal programma e `mustShutdown` verrà scritta solo una volta dal thread gestore.

## Documentazione  
Tutto il programma contiene commenti che possono essere usati per generare documentazione automaticamente. In particolare, i file C usano commenti in formato `Doxygen`, mentre i file Java utilizzano commenti in formato `JavaDocs`.
