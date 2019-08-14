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

#include <lib.h>
#include <thread_worker.h>
#include <server.h>

#define target_output stdout


//Procedura che invia come risposta 'OK' al client e stampa alcune informazioni utili a schermo
void sendOK(int connfd, char* currentUser, char* operation){
  writen(connfd,"OK \n",4);
  fprintf(target_output,"user %-15s fd: %-10dop: %-10s\tOK \n",  currentUser, connfd, operation);
}

//Stessa cosa con 'KO', qui però devo devo formattare diversamente la risposta
void sendKO(int connfd, char* currentUser, char* operation, char* message){
  char* toPrint = NULL;

  //Posso decidere se inviare insieme a 'KO' un messaggio personalizzato sul momento
  //oppure uno standard (KO + errno in versione testuale)
  if(message == NULL)
    toPrint = strerror(errno);
  else toPrint = message;

  //Preparo il buffer per formattare la stringa
  char fail_buf[MAX_RESPONSE_SIZE];
  memset(fail_buf,'\0',MAX_RESPONSE_SIZE);

  //Formatto la stringa usando il valore settato precedentemente a seconda dei due casi
  sprintf(fail_buf,"KO [%s] \n", toPrint);

  //Invio il KO 'completo'
  writen(connfd,fail_buf,strlen(fail_buf));

  //Stampo alcune informazioni utili a schermo
  fprintf(target_output,"user %-15s fd: %-10dop: %-10s\t%s",currentUser, connfd,operation,fail_buf);
}

//Procedura che si occupa della disconnessione dell'utente
void leave(int connfd, char* currentUser){
  //Vado a rimuovere l'utente dalla tabella hash, in questo modo do la possibilita'
  //all'utente di riconnettersi (se lo desidera) in un secondo momento
  removeHashTable(&HT,currentUser);
  memset(currentUser, '\0', USER_MAX_LENGTH);
  //Mando la risposta (positiva) al client
  sendOK(connfd, currentUser, "LEAVE");
}


//Procedura che si occupa della registrazione dell'utente
void register_(int connfd, char* currentUser, char* name){
  //Se è già presente nella tabella hash, vuol dire che c'è un altro client
  //attualmente connesso con lo stesso username: la procedura fallisce mandando KO
  if(isInHashTable(HT,name)){
    sendKO(connfd,name,"REGISTER","Multiple clients with the same username");
    return;
  }


  //Se l'utente non è stato ancora registrato, posso allora procedere con
  //la registrazione (e conseguente inserimento nella tabella)
  if(currentUser[0] == '\0'){
    strcpy(currentUser,name);
    insertHashTable(&HT,currentUser);
  }

/////////////////////////////////////////////////////////////////TODO ELSE///////////////////////////////////////////


    //Ottengo il path relativo all'utente
    char* user_path = getUserPath(name);

    //Provo a creare la direcytory
    int result = mkdir(user_path,  0755);
    free(user_path);

    //Se ho avuto successo nella creazione, mando OK
    if(result == 0)
      sendOK(connfd, currentUser, "REGISTER");
    else{
      //Se fallisco nella creazione, ma è perché la directory esiste già, va tutto bene
      //(l'utente si era già registrato in precedenza)
      if(errno == EEXIST)
        sendOK(connfd, currentUser, "REGISTER");
      //Se fallisco in ogni altro caso, vuol dire che ho avuto un problema e mando KO
      else
        sendKO(connfd, currentUser, "REGISTER", NULL);
    }

}

void store(int connfd, char* currentUser ,char* name, long int len, char* newline){
  void *data = calloc(len,1);
  int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);

  int n;
  if(len-b > 0){
    memcpy(data,(newline+2),b);
    n = readn(connfd, ((char*) data)+b,len-b);
    if(n <= 0) return;
  }
  else memcpy(data,(void*)(newline+2),len);

  if(currentUser[0] == '\0')
    sendKO(connfd, currentUser, "STORE", "User not registered");
  else{
      if(createFile(name,data,currentUser,len))
        sendOK(connfd, currentUser, "STORE");
      else
        sendKO(connfd, currentUser, "STORE", NULL);
  }

  free(data);
}

void retrieve(int connfd, char* currentUser, char* name){

  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,name);

  FILE *f = fopen(file_path, "rb");

  if(f){
    int d = fileno(f);

    struct stat finfo;
    fstat(d, &finfo);

    size_t size = finfo.st_size;
    void *block = calloc(size, 1);

    fread(block, size, 1, f);

    int N = 8 + getNumberOfDigits(size);
    char* buff = calloc(N + 1, sizeof(char));

    sprintf(buff,"DATA %lu \n ", size);

    char* tmp = calloc(N+1+size,sizeof(char));
    memcpy(tmp,buff,N);
    memcpy(tmp+N,block,size);

    writen(connfd,tmp,N+size);
    fprintf(target_output,"user %-15s fd: %-10dop: %-10s\tOK \n",  currentUser, connfd, "RETRIEVE");

    free(buff);
    free(tmp);
    free(block);
    fclose(f);
 	  }
    else
      sendKO(connfd, currentUser, "RETRIEVE", "Can't retrieve the object");

  free(user_path);
  free(file_path);

}


void delete(int connfd, char* currentUser, char* obj_name){
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(obj_name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,obj_name);

  bool success = (remove(file_path) == 0);

  if(success) sendOK(connfd, currentUser, "DELETE");
  else sendKO(connfd, currentUser, "DELETE", NULL);

  free(user_path);
  free(file_path);
}


//Procedura relativa al parsing delle richieste
void parse_request(int connfd, char *str, char* currentUser){
 if(str == NULL || str[0] == '\0')return;

 //Puntatore 'token' necessario per la strtok
 char* ptr = NULL;
 long int len = 0;

 //Spezzo la stringa in funzione del protocollo
 char* operation = strtok_r(str, " ", &ptr);
 if(operation == NULL)return;
 char* name = strtok_r(NULL, " ", &ptr);
 char* len_str = strtok_r(NULL, " ", &ptr);
 char* newline = strtok_r(NULL, " ", &ptr);

 //Converto la stringa che descrive la lunghezza in un numero
 if(len_str != NULL)
  len = atol(len_str);


 //Per ogni operazione decodificata vado ad eseguire la rispettiva procedura che
 //si occupa di fare tutto il necessario
 if(str_equals(operation, "REGISTER"))
  register_(connfd, currentUser, name);
 else if(str_equals(operation, "STORE"))
  store(connfd, currentUser, name, len, newline);
 else if(str_equals(operation, "RETRIEVE"))
  retrieve(connfd, currentUser, name);
 else if(str_equals(operation, "DELETE"))
  delete(connfd, currentUser, name);
 else if(str_equals(operation, "LEAVE"))
  leave(connfd, currentUser);

}
