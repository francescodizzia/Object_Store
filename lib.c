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
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>

#include <common.h>

//Valore di default del fd globale
int fd = -1;

//Stringa contenente l'ultimo errore riscontrato dopo un'operazione, viene
//opportunamente settata nelle varie funzioni (os_register, os_store, etc...)
char last_error_msg[256];

//Stampa l'ultimo messaggio d'errore (utile durante i test)
void printLastErrorMsg(){
  printf("ERROR: %s\n", last_error_msg);
  memset(last_error_msg, '\0', 256);
}


//Legge il buffer di risposta, restituisce TRUE se è un OK, altrimenti FALSE
bool getResponseMsg(){
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);
  read(fd,response_buf,MAX_RESPONSE_SIZE);

  if(strncmp("OK",response_buf,2) == 0)
   return true;
  else{
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, response_buf);
  }

 return false;
}

//Leggo il buffer di risposta che viene inviato dal server nel caso specifico
//della retrieve
void *getDataResponseMsg(){
  //Preparo il buffer
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);

  //Vado a leggere la risposta e a metterla nel buffer
  read(fd,response_buf, MAX_RESPONSE_SIZE);

  //Preparo il necessario per tokenizzare
  char* ptr = NULL;
  long int len = -1;
  char* first_str =  strtok_r(response_buf, " ", &ptr);

  //Se ho avuto dei problemi a tokenizzare, oppure
  //la risposta è di tipo KO, setto il messaggio di errore
  //e ritorno NULL
  if(first_str == NULL || str_equals(first_str,"KO")){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Can't retrieve the object]");
    return NULL;
  }

  //Tokenizzo la lunghezza e il newline
  char* len_str =  strtok_r(NULL, " ", &ptr);
  char* newline = strtok_r(NULL, " ", &ptr);

  if(len_str != NULL) len = atol(len_str);

  //Lunghezza non valida
  if(len < 0){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Object length isn't valid]\n");
    return NULL;
  }

  //Preparo il buffer con i dati
  void *data = calloc(len,1);

  //In modo simmetrico alla store (del parser), vado a calcolare la quantità
  //di dati già letti dalla prima read
  int r = MAX_RESPONSE_SIZE-DATA_MSG_LENGTH-getNumberOfDigits(len);

  int n;

  //La differenza tra la lunghezza e i dati letti è maggiore di zero: devo
  //fare una ulteriore read di lunghezza 'len-r' per finire la lettura dei dati
  if(len-r > 0){
    //Copio i dati parziali nel buffer
    memcpy(data,(newline+2),r);

    //Completo la lettura e metto il tutto nel buffer
    n = readn(fd, ((char*) data)+r,len-r);

    //Errore: ritorno NULL
    if(n <= 0)return NULL;
  }
  //Se avevo già completato con la prima read, posso direttamente copiare nel
  //buffer il contenuto
  else memcpy(data,(void*)(newline+2),len);


 //Ritorno i dati
 return data;
}


//Connessione all'Object Store, con un determinato nome utente
int os_connect(char *name) {
  //Se il file descriptor non è stato resettato, vuol dire che il client è attualmente connesso
  //e sta tentando di registrarsi ulteriormente (cosa non permessa)
  if(fd != -1){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Client already connected, need to disconnect first]");
    return false;
  }

  //Inizializzo nel modo classico il fd e socket
  struct sockaddr_un serv_addr;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

  //Tento la connessione, in caso di fallimento ritorno FALSE
  if(connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    return false;

  //Calcolo la dimensione necessaria per il buffer
  int len = strlen(name);
  int N = len + REGISTER_LENGTH;

  //Creo il buffer, lo imposto secondo le regole dettate dal protocollo
  char* buff = calloc(N+1, sizeof(char));
  sprintf(buff,"REGISTER %s \n",name);

  //Provo a inviare il messaggio
	int n = writen(fd, buff, N);

  //Errore nella write: fallimento
  if(n < 0)return false;

  free(buff);

  //Controllo la risposta del server e in base ad essa ritorno successo o fallimento
	return getResponseMsg();
}

//Store di un determinato oggetto, con nome 'name', lunghezza 'len' e contenuto 'block'
int os_store(char *name, void *block, size_t len) {
  //La lunghezza è minore a zero: fallimento
  if(len < 0)return false;

  //Calcolo della dimensione necessaria al buffer per l'invio della richiesta
  size_t store_size = STORE_LENGTH + strlen(name) + getNumberOfDigits(len);

  //Crezione del buffer
  char* buff = calloc(store_size+1, sizeof(char));
  if(!buff)return false;

  //Formatto il buffer secondo le regole del protocollo
  sprintf(buff,"STORE %s %lu \n ",name, len);

  //Creo il buffer 'definitivo' che andrà inviato realmente
  char* final = calloc(store_size+1+len, sizeof(char));
  //Copio il contenuto parziale dal vecchio buffer nel nuovo
  memcpy(final,buff,store_size);
  //Attraverso l'aritmetica dei puntatori riesco a copiare nel punto esatto il 'block'
  //all'interno del nuovo buffer
  memcpy(final+store_size,block,len);

  //Invio il messaggio completo
  int w = writen(fd,final,store_size+len);
  //Errore nella write: fallimento
  if(w == -1)return false;

  //Procedo col svuotare i buffer
  free(buff);
  free(final);

  //Controllo la risposta del server e decido se restituire successo o meno
	return getResponseMsg();
}


//Esegue il retrieve di un oggetto, dato il nome
//Restituisce un puntatore al blocco dati dell'oggetto, oppure NULL in caso di fallimento
void *os_retrieve(char* name){
  //Solito calcolo della dimensione e creazione del buffer
  int N = strlen(name) + RETRIEVE_LENGTH;
  char* buff = calloc(N+1, sizeof(char));
  sprintf(buff, "RETRIEVE %s \n", name);
  if(!buff) return NULL;

  //Provo a mandare il messaggio
  int w = writen(fd, buff, N);
  free(buff);

  //Fallimento nella write, ritorno NULL
  if(w == -1)return NULL;

  //Procedo col verificare, mediante una funzione ausiliaria, l'esito
  //dell'operazione ed eventualmente restituisco il blocco dati
  return getDataResponseMsg();
}


//Dato il nome, elimina il rispettivo oggetto
int os_delete(char* name){
  //Ancora una volta, calcolo della dimensione del buffer, formattazione
  //via sprintf e scrittura
  int N = strlen(name) + DELETE_LENGTH;
  char* buff = calloc(N+1, sizeof(char));
  sprintf(buff,"DELETE %s \n", name);

  int w = writen(fd, buff, N);
  free(buff);

  //La write fallisce: ritorno FALSE
  if(w == -1)return false;

  //Verifico l'esito dell'operazione e restituisco successo o fallimento
  return getResponseMsg();
}


//Esegue la disconnessione dell'utente corrente
int os_disconnect(){
 //In questo caso non ho bisogno di calcolare dimensioni o allocare buffer,
 //perché il messaggio è costante, con una dimensione fissa
 int w = writen(fd, "LEAVE \n", 7);
 //Fallimento nella write
 if(w == -1)return false;

 //Verifico e memorizzo in una variabile il responso, devo fare così
 //perché se lo facessi alla fine, come nelle altre funzioni, in questo caso
 //avrei un problema: il file descriptor verrebbe resettato prima e non avrei
 //modo di verificare la risposta, quindi decido prima di verificare la risposta,
 //chiudere il file descriptor, resettarlo, vedere eventuali errori e successivamente
 //guardare il responso che era stato memorizzato nella variabile booleana
 bool response = getResponseMsg();
 int c = close(fd);
 fd = -1;

 if(c == -1)return false;

 return response;
}
