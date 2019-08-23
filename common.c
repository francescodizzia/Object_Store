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

 //Apro il file, creandolo se non esiste, se è già esistente viene 'troncato' attraverso il flag O_TRUNC
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
