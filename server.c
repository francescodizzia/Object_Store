#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <ftw.h>

#include <common.h>
#include <thread_worker.h>
#include <hashtable.h>


//Dimensione della tabella hash
#define HASH_TABLE_SIZE 512

//1 MB = 1000000 byte
#define ONE_MB 1000000


//Flag globale, condiviso tra il server e i vari client: quando viene settato
//a false avvia la terminazione 'gentile' dell'Object Store e dei vari thread
volatile sig_atomic_t running = true;

//Variabile che rappresenta il numero di client attualmente connessi al server
volatile sig_atomic_t n_clients = 0;

//Mutex riguardante la variabile n_clients
pthread_mutex_t client_mtx = PTHREAD_MUTEX_INITIALIZER;

//Variabile di condizione sulla quale si sospende il server mentre è in attesa
//che tutti i client vengano chiusi
pthread_cond_t empty;

//Rappresenta il set dei segnali
sigset_t set;

//Tabella Hash, necessaria per garantire che non vi siano utenti collegati con più
//client contemporaneamente (ulteriori info nella relazione)
hashtable HT;

//Variabili riguardanti le statistiche che vengono settate e stampate (insieme al numero
//di client attualmente connessi) dopo che il server riceve un segnale di tipo 'SIGUSR1'
size_t total_size = 0;  //Dimensione totale (in byte) dello Store

size_t objects = 0;     //Numero di oggetti immagazzinati

size_t folders = -1;    //Numero di cartelle (e quindi, di utenti che si sono connessi almeno
                        //una volta allo Store), parte da -1 per escludere la
                        //cartella 'data' stessa dal conteggio


//Funzione ausiliaria che, attraverso la ftw (file-tree-walk) si occupa di settare opportunamente
//le statistiche elencate sopra
int setStats(const char* filename, const struct stat* stats, int type){
  if(type == FTW_F) //Ho trovato un file, incremento il contatore degli oggetti
    objects++;
  else if(type == FTW_D) //Ho trovato una cartella, incremento il relativo contatore
    folders++;

  //Incremento la size totale dello Store
  total_size += stats->st_size;
  return 0;
}

//Resetta semplicemente le statistiche
void resetStats(){
  total_size = 0;
  objects = 0;
  folders = -1;
}


//Si occupa della stampa delle statistiche
void printStats(){
  //Esegue la file-tree-walk sulla directory 'data', con il criterio specificato nella funzione
  //setStats, vista sopra
  int k = ftw("./data/", setStats, 0);

  //In caso di errore, resetto le statistiche ed esco senza stampare nulla
  if(k == -1){resetStats();return;}

  //Converto il numero di total_size (che è in byte) in MB
  float size_in_MB = ((float)total_size)/ONE_MB;

  //Alloco e inizializzo la stringa che stamperà le varie informazioni
  char string[512];
  memset(string, '\0',512);

  //Dato che andrò ad accedere sulla variabile condivisa n_clients, acquisisco il lock
  //relativo alla sezione critica prima
  pthread_mutex_lock(&client_mtx);
    //Formatto la stringa secondo le varie informazioni
    sprintf(string,"\n**************************************************\nSize totale degli oggetti: %lu byte (%.2f MB)\nNumero di oggetti: %lu\nCartelle: %lu\nClient connessi: %d\n**************************************************\n",total_size, size_in_MB, objects,folders,n_clients);
    //Stampo a schermo la stringa, usando una write anziché una printf per questioni di signal safety
    write(1,string,strlen(string));
    //Resetto le statistiche, pronte per una nuova stampa
    resetStats();
  //Rilascio il lock
  pthread_mutex_unlock(&client_mtx);
}


//Crea un thread worker destinato a servire il client appena connesso
void spawnThreadWorker(long connfd){
  pthread_attr_t thattr;
  pthread_t thid;

  //Provo a inizializzare gli attributi del thread
  if(pthread_attr_init(&thattr) != 0){
	    printf("pthread_attr_init FALLITA\n");
	    close(connfd);
	    return;
  }

  //Tento di settare il thread in modalità 'detached'
  if(pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0){
	   printf("pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
     close(connfd);
     return;
  }

  //Provo a creare il thread worker (vedere il file 'thread_worker.c')
  if(pthread_create(&thid, &thattr, thread_worker, (void*)connfd) != 0){
     printf("pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   close(connfd);
	   return;
  }
}


//Thread handler dei segnali
void* signal_handler(void* ptr){
    int signal;

    //Finché il server è in esecuzione, il thread dei segnali aspetta attraverso
    //sigwait l'arrivo di un segnale
    while(running){
        //Aspetto il segnale, una volta arrivato lo salvo nella variabile 'signal'
        sigwait(&set, &signal);

        //Se il segnale è di tipo SIGUSR1, procedo con la stampa delle statistiche
        if(signal == SIGUSR1)
          printStats();
        //Ogni altro tipo di segnale setta la variabile running a FALSE, procedendo
        //quindi alla terminazione 'gentile' del server, dei thread worker e dello
        //stesso thread dei segnali
        else
          running = false;
    }

    return NULL;
}


//Crea il thread che si occuperà di gestire i segnali
void spawnSignalThread(void* ptr){
  pthread_attr_t thattr;
  pthread_t thid;

  //Provo a inizializzare gli attributi del thread
  if(pthread_attr_init(&thattr) != 0){
	   printf("pthread_attr_init FALLITA\n");
	   return;
  }

  //Tento di settare il thread in modalità 'detached'
  if(pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0){
	   printf("pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
     return;
  }

  //Provo a creare il signalThread(vedere la funzione 'signal_handler' definita sopra)
  if(pthread_create(&thid, &thattr, signal_handler, ptr) != 0){
     printf("pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   return;
  }
}


//Classica funzione contenente l'unlink del nome della socket
void cleanup(){
  unlink(SOCKNAME);
}


int main(){

  //Chiamo la funzione di cleanup e faccio in modo che venga chiamata durante
  //la terminazione del programma
  cleanup();
  atexit(cleanup);

  //Creo e inizializzo l'hash table contenente gli utente attualmente connessi
  HT = createHashTable(HASH_TABLE_SIZE);

  //Inizializzo il set dei segnali in modo tale da fargli contenere
  //tutti i segnali
  sigfillset(&set);

  pthread_sigmask(SIG_SETMASK, &set, NULL);

  //Crea il thread destinato alla gestione dei segnali
  spawnSignalThread(&set);

  //Provo a creare la socket
  int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  //In caso di fallimento nella creazione, esco
  if(server_fd == -1){
    printf("Errore nella creazione del socket\n");
    return -1;
  }

  struct sockaddr_un sa;
  strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
  sa.sun_family = AF_UNIX;

  //Effettuo il binding
  if(bind(server_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1){
    printf("Errore nel binding\n");
    return -1;
  }

  //Vado in listening
  if(listen(server_fd, SOMAXCONN) == -1){
    printf("Errore nella listen\n");
    return -1;
  }


  struct timeval timer;
  fd_set fds,r_fds;

  FD_ZERO(&fds);
  FD_SET(server_fd, &fds);

  int select_ret, connfd;

  while(running){
    timer.tv_sec = 0;
    timer.tv_usec = 10000; //10000 microsecondi sono 10 millisecondi
    r_fds = fds;

    //Chiamo la select con timer
    select_ret = select(server_fd+1, &r_fds, NULL, NULL, &timer);

    //Fallimento nella select: esco dal ciclo
    if(select_ret == -1)
	    break;

    //Ci sono dei client da servire
    else if(select_ret > 0){
		  connfd = accept(server_fd, (struct sockaddr*)NULL ,NULL);
      //Procedo con la creazione del thread worker
      spawnThreadWorker(connfd);
    }

  }


  pthread_mutex_lock(&client_mtx);
    if(n_clients > 0){
      printf("Attendo la terminazione degli altri thread...\n");
      pthread_cond_wait(&empty, &client_mtx);
    }
  pthread_mutex_unlock(&client_mtx);

  printf("Tutti i thread sono stati terminati con successo!\n");

  freeHashTable(&HT);
  close(server_fd);

  return 0;
}
