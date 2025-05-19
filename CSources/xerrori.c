#include "../CHeaders/xerrori.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// MESSAGGI D'ERRORE //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Termina un processo con eventuale messaggio d'errore.
 * @details Si noti che la variabile errno è "thread local", quindi ne esiste una diversa per ogni thread.  
 * @param messaggio Messaggio di errore da stampare su stderr.
 */
void termina(const char* messaggio) {
  if (errno == 0) fprintf(stderr, "== %d == %s\n", getpid(), messaggio);
  else fprintf(stderr, "== %d == %s: %s\n", getpid(), messaggio, strerror(errno));
  exit(2);
}

/**
 * @brief Termina un processo stampando un messaggio di errore con parametri variadici, linea e file.
 * @details La variabile errno è "thread_local", quindi ne esiste una diversa per ogni thread.
 * @param linea Numero della linea da cui è stata chiamata la funzione.
 * @param file Nome del file da cui è stata chiamata la funzione.
 * @param formattedMsg Stringa formattata di errore.
 * @param ... Argomenti variadici da "includere" nella stringa formattata.
 */
void xtermina(int linea, char* file, const char* formattedMsg, ...) {
  va_list args;

  fprintf(stderr, "== %d == ", getpid());

  va_start(args, formattedMsg);
  vfprintf(stderr, formattedMsg, args);
  va_end(args);

  if (errno != 0) fprintf(stderr, ": %s", strerror(errno));

  fprintf(stderr, "\n== %d == Linea: %d, File: %s\n", getpid(), linea, file);

  exit(2);
}

#define Buflen 100

/**
 * @brief Stampa il messaggio d'errore associato al codice en senza terminare.
 * @param en Codice dell'errore.
 * @param msg Messaggio da stampare su stderr.
 */
void xperror(int en, char* msg) {
  char buf[Buflen];
  char* errmsg = strerror_r(en, buf, Buflen);
  if(msg != NULL) fprintf(stderr, "%s: %s\n", msg, errmsg);
  else fprintf(stderr, "%s\n", errmsg);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// OPERAZIONI SU FILE /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Apre il file passato nella modalità specificata e stampa eventuali errori.
 * @param path Percorso del file da aprire.
 * @param mode Modalità in cui aprire il file.
 * @param linea Linea dalla quale è chiamata la funzione.
 * @param file File dal quale è chiamata la funzione.
 * @return Puntatore al file aperto.
 */
FILE* xfopen(const char* path, const char* mode, int linea, char* file) {
  FILE* f = fopen(path, mode);
  if(f == NULL) {
    perror("Errore apertura file");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return f;
}

/**
 * @brief Chiude un file descriptor.
 * @param fd File descriptor del file da chiudere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 */
void xclose(int fd, int linea, char* file) {
  int e = close(fd);
  if(e != 0) {
    perror("Errore chiusura file descriptor");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// FUNZIONI PER THREAD ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  NOTA BENE:
    In caso di errore non scrivono il codice d'errore in errno, ma lo restituiscono come return value,
    questo perchè errno non è thread-safe in certe versioni di Linux.

    Return value = 0 => Nessun errore.
*/

/**
 * @brief Crea un thread.
 * @param thread Oggetto pthread_t da creare.
 * @param attr Attributi con cui creare il thread.
 * @param start_routine Puntatore alla funzione che il thread deve eseguire.
 * @param arg Argomenti da passare al thread.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_create.
 */
int xpthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine) (void*), void* arg, int linea, char* file) {
  int e = pthread_create(thread, attr, start_routine, arg);
  if (e != 0) {
    xperror(e, "Errore pthread_create");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;                       
}

/**
 * @brief Aspetta che il thread termini.
 * @details Il thread deve essere joinable. Se è già terminato, ritorna immediatamente.
 * @param thread Thread da aspettare.
 * @param retval Puntatore in cui mettere l'exit status del thread (NULL per ignorare).
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_join().
 */
int xpthread_join(pthread_t thread, void** retval, int linea, char* file) {
  int e = pthread_join(thread, retval);
  if (e != 0) {
    xperror(e, "Errore pthread_join");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// MUTEX /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Inizializza dinamicamente un mutex.
 * @details Inizializzando un mutex dinamicamente, è possibile reinizializzarlo una volta distrutto.
 * @param mutex Puntatore al mutex da inizializzare.
 * @param attr Attributi del mutex (NULL per default).
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_mutex_init().
 */
int xpthread_mutex_init(pthread_mutex_t* restrict mutex, const pthread_mutexattr_t* restrict attr, int linea, char* file) {
  int e = pthread_mutex_init(mutex, attr);
  if (e != 0) {
    xperror(e, "Errore pthread_mutex_init");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }  
  return e;
}

/**
 * @brief Distrugge un mutex.
 * @details Il mutex deve essere sbloccato. Se bloccato, restituisce EBUSY.
 * @param mutex Mutex da distruggere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_mutex_destroy().
 */
int xpthread_mutex_destroy(pthread_mutex_t* mutex, int linea, char* file) {
  int e = pthread_mutex_destroy(mutex);
  if (e != 0) {
    xperror(e, "Errore pthread_mutex_destroy");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Blocca un mutex.
 * @details Se il mutex è già bloccato, il thread chiamante viene sospeso.
 * @param mutex Mutex da bloccare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_mutex_lock().
 */
int xpthread_mutex_lock(pthread_mutex_t* mutex, int linea, char* file) {
  int e = pthread_mutex_lock(mutex);
  if (e != 0) {
    xperror(e, "Errore pthread_mutex_lock");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Sblocca un mutex.
 * @param mutex Mutex da sbloccare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_mutex_unlock().
 */
int xpthread_mutex_unlock(pthread_mutex_t* mutex, int linea, char* file) {
  int e = pthread_mutex_unlock(mutex);
  if (e != 0) {
    xperror(e, "Errore pthread_mutex_unlock");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// BARRIERE ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Inizializza una barriera.
 * @param barrier Barriera da inizializzare.
 * @param attr Attributi della barriera (NULL per default).
 * @param count Numero di thread necessari per sbloccare la barriera.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_barrier_init().
 */
int xpthread_barrier_init(pthread_barrier_t* restrict barrier, const pthread_barrierattr_t* restrict attr, unsigned int count, int linea, char* file) {
  int e = pthread_barrier_init(barrier, attr, count);
  if (e != 0) {
    xperror(e, "Errore pthread_barrier_init");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }  
  return e;
}

/**
 * @brief Distrugge una barriera.
 * @param barrier Barriera da distruggere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_barrier_destroy().
 */
int xpthread_barrier_destroy(pthread_barrier_t* barrier, int linea, char* file) {
  int e = pthread_barrier_destroy(barrier);
  if (e != 0) {
    xperror(e, "Errore pthread_barrier_destroy");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Attende che la barriera venga raggiunta.
 * @param barrier Barriera su cui attendere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return PTHREAD_BARRIER_SERIAL_THREAD per un thread, 0 per gli altri.
 */
int xpthread_barrier_wait(pthread_barrier_t* barrier, int linea, char* file) {
  int e = pthread_barrier_wait(barrier);
  if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD) {
    xperror(e, "Errore pthread_barrier_wait");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// CONDITION VARIABLES ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Inizializza una condition variable.
 * @param cond Condition variable da inizializzare.
 * @param attr Attributi (NULL per default).
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_cond_init().
 */
int xpthread_cond_init(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr, int linea, char* file) {
  int e = pthread_cond_init(cond,attr);
  if (e != 0) {
    xperror(e, "Errore pthread_cond_init");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Distrugge una condition variable.
 * @param cond Condition variable da distruggere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_cond_destroy().
 */
int xpthread_cond_destroy(pthread_cond_t* cond, int linea, char* file) {
  int e = pthread_cond_destroy(cond);
  if (e != 0) {
    xperror(e, "Errore pthread_cond_destroy");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Attende su una condition variable.
 * @param cond Condition variable su cui attendere.
 * @param mutex Mutex associato alla condition variable.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_cond_wait().
 */
int xpthread_cond_wait(pthread_cond_t* restrict cond, pthread_mutex_t* restrict mutex, int linea, char* file) {
  int e = pthread_cond_wait(cond, mutex);
  if (e != 0) {
    xperror(e, "Errore pthread_cond_wait");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Segnala a un thread in attesa su una condition variable.
 * @param cond Condition variable da segnalare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_cond_signal().
 */
int xpthread_cond_signal(pthread_cond_t* cond, int linea, char* file) {
  int e = pthread_cond_signal(cond);
  if (e != 0) {
    xperror(e, "Errore pthread_cond_signal");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}

/**
 * @brief Segnala a tutti i thread in attesa su una condition variable.
 * @param cond Condition variable da segnalare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pthread_cond_broadcast().
 */
int xpthread_cond_broadcast(pthread_cond_t* cond, int linea, char* file) {
  int e = pthread_cond_broadcast(cond);
  if (e != 0) {
    xperror(e, "Errore pthread_cond_broadcast");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// SEMAFORI POSIX ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  NOTA BENE:
    I semafori POSIX sono usati sia da processi che da thread, è utile distinguere tra l'utilizzo di exit() e pthread_exit().

    Threads:
      - Chiusura con exit() -> Causa la terminazione del processo intero.
      - Chiusura con pthread_exit() -> Causa la terminazione del singolo thread.
    Processi:
      - Chiusura con exit() -> Causa la terminazione del processo intero, necessario per stampare errore.
    
    E' possibile distinguere se quello in esecuzione è un thread o un processo utilizzando gettid(),
    se il valore coincide con il PID principale, il codice sta girando su un processo, altrimenti su un thread.
*/

/**
 * @brief Apre/crea un semaforo POSIX named.
 * @param name Nome del semaforo.
 * @param oflag Flag di apertura (O_CREAT, O_EXCL, etc.).
 * @param mode Permessi del semaforo.
 * @param value Valore iniziale del semaforo.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Puntatore al semaforo, SEM_FAILED in caso di errore.
 */
sem_t* xsem_open(const char* name, int oflag, mode_t mode, unsigned int value, int linea, char* file) {
  sem_t* s = sem_open(name, oflag, mode, value);
  if (s == SEM_FAILED) {
    perror("Errore sem_open");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file); 
    exit(2);
  }
  return s;
}

/**
 * @brief Chiude un semaforo POSIX named.
 * @param s Semaforo da chiudere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_close().
 */
int xsem_close(sem_t* s, int linea, char* file) {
  int e = sem_close(s);
  if(e != 0) {
    perror("Errore sem_close"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;  
}

/**
 * @brief Rimuove un semaforo POSIX named.
 * @param name Nome del semaforo da rimuovere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_unlink().
 */
int xsem_unlink(const char* name, int linea, char* file) {
  int e = sem_unlink(name);
  if(e != 0) {
    perror("Errore sem_unlink"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;  
}

/**
 * @brief Inizializza un semaforo POSIX unnamed.
 * @param sem Semaforo da inizializzare.
 * @param pshared Flag di condivisione tra processi (0 = no, 1 = si).
 * @param value Valore iniziale del semaforo.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_init().
 */
int xsem_init(sem_t* sem, int pshared, unsigned int value, int linea, char* file) {
  int e = sem_init(sem, pshared, value);
  if(e != 0) {
    perror("Errore sem_init"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}

/**
 * @brief Distrugge un semaforo POSIX unnamed.
 * @param sem Semaforo da distruggere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_destroy().
 */
int xsem_destroy(sem_t* sem, int linea, char* file) {
  int e = sem_destroy(sem);
  if(e != 0) {
    perror("Errore sem_destroy"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}

/**
 * @brief Incrementa un semaforo (operazione V).
 * @param sem Semaforo da incrementare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_post().
 */
int xsem_post(sem_t* sem, int linea, char* file) {
  int e = sem_post(sem);
  if(e != 0) {
    perror("Errore sem_post"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}

/**
 * @brief Decrementa un semaforo (operazione P).
 * @param sem Semaforo da decrementare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di sem_wait().
 */
int xsem_wait(sem_t* sem, int linea, char* file) {
  int e = sem_wait(sem);
  if(e != 0) {
    perror("Errore sem_wait"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// OPERAZIONI SU PROCESSI //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Crea un nuovo processo figlio.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return PID del figlio (0 nel figlio, >0 nel padre).
 */
pid_t xfork(int linea, char* file) {
  pid_t p = fork();
  if(p < 0) {
    perror("Errore fork");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return p;
}

/**
 * @brief Attende la terminazione di un processo figlio.
 * @param status Puntatore per lo stato di uscita (NULL per ignorare).
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return PID del figlio terminato.
 */
pid_t xwait(int* status, int linea, char* file) {
  pid_t p = wait(status);
  if(p < 0) {
    perror("Errore wait");
    fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);
    exit(2);
  }
  return p;
}

/**
 * @brief Crea una pipe.
 * @param pipefd Array di 2 file descriptor per la pipe.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di pipe().
 */
int xpipe(int pipefd[2], int linea, char* file) {
  int e = pipe(pipefd);
  if(e!=0) {
    perror("Errore creazione pipe"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}

// ---------------- memoria condivisa POSIX

/**
 * @brief Apre/crea un segmento di memoria condivisa.
 * @param name Nome del segmento.
 * @param oflag Flag di apertura.
 * @param mode Permessi del segmento.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return File descriptor del segmento.
 */
int xshm_open(const char* name, int oflag, mode_t mode, int linea, char* file) {
  int e = shm_open(name, oflag, mode);
  if(e== -1) {
    perror("Errore shm_open"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;  
}

/**
 * @brief Rimuove un segmento di memoria condivisa.
 * @param name Nome del segmento da rimuovere.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di shm_unlink().
 */
int xshm_unlink(const char* name, int linea, char* file) {
  int e = shm_unlink(name);
  if(e== -1) {
    perror("Errore shm_unlink"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;  
}

/**
 * @brief Imposta la dimensione di un file/segmento.
 * @param fd File descriptor.
 * @param length Nuova dimensione.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di ftruncate().
 */
int xftruncate(int fd, off_t length, int linea, char* file) {
  int e = ftruncate(fd,length);
  if(e== -1) {
    perror("Errore ftruncate"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;  
}

/**
 * @brief Mappa un file/segmento in memoria.
 * @param length Dimensione della mappatura.
 * @param fd File descriptor da mappare.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Puntatore alla memoria mappata.
 */
void* simple_mmap(size_t length, int fd, int linea, char* file) {
  void* a =  mmap(NULL, length ,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  if(a == (void*) -1) {
    perror("Errore mmap"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return a;
}

/**
 * @brief Rimuove una mappatura di memoria.
 * @param addr Indirizzo della mappatura.
 * @param length Dimensione della mappatura.
 * @param linea Numero della linea da cui è chiamata la funzione.
 * @param file Nome del file da cui è chiamata la funzione.
 * @return Codice d'errore di munmap().
 */
int xmunmap(void* addr, size_t length, int linea, char* file) {
  int e = munmap(addr, length);
  if(e== -1) {  
    perror("Errore munmap"); 
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(2);
  }
  return e;
}