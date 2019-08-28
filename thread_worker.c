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
#include <stdbool.h>

#include <common.h>
#include <server.h>
#include <parser.h>
#include <hashtable.h>
#include <thread_worker.h>


//Cambio il fd in stato 'blocking' o 'unblocking' in funzione
//della variabile booleana blocking
void setBlockingFD(int connfd, bool blocking){
  int flags;

  //Metto il fd in stato 'bloccante'
  if(blocking){
    flags = fcntl(connfd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(connfd, F_SETFL, flags);
  }
  //Metto il fd in stato 'unblocking'
  else{
    flags = fcntl(connfd, F_GETFL, 0);
    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
  }

}


void *thread_worker(void *arg) {
  long connfd = (long)arg;

  //Imposto il file descriptor come 'unblocking'
  setBlockingFD(connfd, false);

  //Alloco header e la stringa che mi andra' a rappresentare l'utente attualmente connesso
  char *header = calloc(MAX_HEADER_SIZE, sizeof(char));
  char currentUser[USER_MAX_LENGTH];
  memset(currentUser,'\0',USER_MAX_LENGTH);

  //Incremento il numero di client connessi (sezione critica)
  pthread_mutex_lock(&client_mtx);
  n_clients++;
  pthread_mutex_unlock(&client_mtx);

  int u;

  //Loop del thread: ogni volta che mi arriva una richiesta vado ad effettuare il parsing
  //di tale richiesta ed eseguo le corrispettive operazioni
  while(running){
    //Leggo l'header
    u = read(connfd, header, MAX_HEADER_SIZE);

    //Nel caso in cui ci sia un errore nella lettura dell'header o un fallimento nella
    //comunicazione via socket (magari il client relativo al thread si è disconnesso)
    //esco dal loop, se invece la risorsa non è disponibile ma il flag è ancora settato
    //a true, continuo a iterare
    if(u == -1){
      if(errno == EAGAIN && running)
        continue;
      else break;
    }
    if(u == 0)break;

    //Effettuo il parsing e l'eventuale esecuzione dell'operazione richiesta
    parse_request(connfd, header, currentUser);
    //Resetto l'header
    memset(header, '\0', MAX_HEADER_SIZE);
  }

  //Vado a rimuovere l'utente dalla tabella hash, in questo modo do la possibilità
  //all'utente di riconnettersi (se lo desidera) in un secondo momento
  if(currentUser[0] != '\0'){
    removeHashTable(&HT, currentUser);
    memset(currentUser, '\0', USER_MAX_LENGTH);
  }


  //Decremento il numero di client connessi
  pthread_mutex_lock(&client_mtx);
    n_clients--;

    //Se quest'ultimo utente che ho rimosso era l'ultimo, posso svegliare
    //il server, che era in attesa fino a questo momento
    if(n_clients <= 0)
      pthread_cond_signal(&empty);
  pthread_mutex_unlock(&client_mtx);

  free(header);
  close(connfd);

  return NULL;
}
