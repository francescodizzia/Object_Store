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

#include <lib.h>
#include <hashtable.h>


#define REGISTER_LENGTH 11
#define STORE_LENGTH 10
#define RETRIEVE_LENGTH 11
#define DELETE_LENGTH 9
#define DATA_MSG_LENGTH 8

//Valore di default del fd globale
int fd = -1;

//Stringa contenente l'ultimo errore riscontrato dopo un'operazione, viene
//opportunamente settata nelle varie funzioni (os_register, os_store, etc...)
char last_error_msg[256];

//Funzione che, dato un intero, restituisce il numero di cifre
//(Si è preferito creare una funzione ad-hoc anziché fare una cosa del
//tipo 'convertire l'intero in stringa e fare una strlen' per ragioni di
//performance e leggibilità)
size_t getNumberOfDigits(size_t k){
  int len;
	if(k == 0)return 1;

  for(len = 0; k > 0; len++)
    k = k/10;

	return len;
}

//Restituisce il path (sotto forma di stringa) relativo all'user
char* getUserPath(char* username){
  char* path = calloc(MAX_PATH_SIZE, sizeof(char));
  sprintf(path,"%s%s/",DATA_DIRECTORY, username);

  return path;
}


//Crea un file, di grandezza 'size', con contenuto 'data'
bool createFile(char* filename, void* data, char* username, size_t size){
 //Ottengo il path relativo al file
 char *path;
 path = getUserPath(username);
 strcat(path,filename);

 //'Apro' il file, creandolo se non esiste, se è già esistente viene 'troncato' attraverso il flag O_TRUNC
 int new_fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

 //Errore nella open, esco
 if(new_fd < 0)
  return false;

 //Attraverso la write scrivo il contenuto del buffer 'data' nel file appena creato
 int w = writen(new_fd, data, size);

 //Errore nella write, esco
 if(w == -1)
  return false;

 free(path);
 close(new_fd);
 return true;
}

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


void *getDataResponseMsg(){
  char response_buf[MAX_RESPONSE_SIZE];

  memset(response_buf, '\0', MAX_RESPONSE_SIZE);
  read(fd,response_buf,MAX_RESPONSE_SIZE);

  char* ptr = NULL;
  long int len = -1;
  char* first_str =  strtok_r(response_buf, " ", &ptr);

  if(first_str == NULL || str_equals(first_str,"KO")){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Can't retrieve the object]");
    return NULL;
  }

  char* len_str =  strtok_r(NULL, " ", &ptr);
  char* newline = strtok_r(NULL, " ", &ptr);

  if(len_str != NULL) len = atol(len_str);

  if(len == 0){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Object length is equal to zero]\n");
    return NULL;
  }

  void *data = calloc(len,1);
  int b = MAX_RESPONSE_SIZE-DATA_MSG_LENGTH-getNumberOfDigits(len);

  int n;
  if(len-b > 0){
    memcpy(data,(newline+2),b);
    n = readn(fd, ((char*) data)+b,len-b);
    if(n <= 0)return NULL;
  }
  else memcpy(data,(void*)(newline+2),len);

 return data;
}


//Connessione all'ObjectStore, con un determinato nome utente
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
  int N = len + REGISTER_LENGTH + 1;

  //Creo il buffer, lo imposto secondo le regole dettate dal protocollo
  char* buff = calloc(N, sizeof(char));
  sprintf(buff,"REGISTER %s \n",name);

  //Provo a inviare il messaggio
	int n = write(fd, buff, N-1);

  //Errore nella write: fallimento
  if(n < 0)return false;

  free(buff);

  //Controllo la risposta del server e in base ad essa ritorno successo o fallimento
	return getResponseMsg();
}

//Store di un determinato oggetto, con nome 'name', lunghezza 'len' e contenuto 'block'
int os_store(char *name, void *block, size_t len) {
  //Il puntatore al blocco è nullo o la lunghezza è minore o uguale a zero: fallimento
  if(block == NULL || len <= 0 )return false;

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
  int N = strlen(name) + RETRIEVE_LENGTH + 1;
  char* buff = calloc(N, sizeof(char));
  sprintf(buff, "RETRIEVE %s \n", name);
  if(!buff) return NULL;

  //Provo a mandare il messaggio
  int w = writen(fd, buff, N-1);
  free(buff);

  //Fallimento nella write, ritorno NULL
  if(w == -1)return NULL;

  printf("ciao %s\n",name);
  //Procedo col verificare, mediante una funzione ausiliaria, l'esito
  //dell'operazione ed eventualmente restituisco il blocco dati
  return getDataResponseMsg();
}


//Dato il nome, elimina il rispettivo oggetto
int os_delete(char* name){
  //Ancora una volta, calcolo della dimensione del buffer, formattazione
  //via sprintf e scrittura
  int N = strlen(name) + DELETE_LENGTH + 1;
  char* buff = calloc(N, sizeof(char));
  sprintf(buff,"DELETE %s \n", name);

  int w = writen(fd, buff, N-1);
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


//Una normale strcmp, un po' più comoda da utilizzare
bool str_equals(char* a, char* b){
 if(a == NULL && b == NULL)
  return true;

 if(a == NULL || b == NULL)
  return false;

 return (strcmp(a,b) == 0);
}


//Funzione readn, vista a lezione
int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

  while(left > 0){
	   if((r = read((int)fd ,bufptr,left)) == -1) {
	     if (errno == EINTR) continue;
      return -1;
	   }

     if (r == 0) return 0;

     left -= r;
	   bufptr += r;
  }

  return size;
}


//Funzione writen, vista a lezione
int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

    while(left > 0) {
	    if((r = write((int)fd ,bufptr,left)) == -1) {
	        if (errno == EINTR) continue;
	    return -1;
	    }

      if(r == 0) return 0;

      left -= r;
	    bufptr += r;
    }

    return 1;
}
