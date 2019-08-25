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
#include <thread_worker.h>
#include <server.h>

#define target_output stdout

//Procedura che invia come risposta 'OK' al client e stampa alcune informazioni utili a schermo
void sendOK(int connfd, char* currentUser, char* operation, char* obj_name){
  writen(connfd,"OK \n",4);
  fprintf(target_output,"%-15s %-10s\tfd: %-10dop: %-10s\tOK \n",  currentUser, obj_name, connfd, operation);
}

//Stessa cosa con 'KO', qui però devo devo formattare diversamente la risposta
void sendKO(int connfd, char* currentUser, char* operation, char* obj_name, char* message){
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
//  fprintf(target_output,"user: %-15s fd: %-10dop: %-10s\t%s",currentUser, connfd,operation,fail_buf);
  fprintf(target_output,"%-15s %-10s\tfd: %-10dop: %-10s\t%s",  currentUser, obj_name, connfd, operation, fail_buf);
}


//Procedura che si occupa della disconnessione dell'utente
void leave(int connfd, char* currentUser){
  //Vado a rimuovere l'utente dalla tabella hash, in questo modo do la possibilità
  //all'utente di riconnettersi (se lo desidera) in un secondo momento
  removeHashTable(&HT,currentUser);
  char* exUser = strdup(currentUser);
  memset(currentUser, '\0', USER_MAX_LENGTH);

  //Mando la risposta (positiva) al client
  sendOK(connfd, exUser, "LEAVE", NULL);
  free(exUser);
}


//Procedura che si occupa della registrazione dell'utente
void register_(int connfd, char* currentUser, char* name){
  //Se è già presente nella tabella hash, vuol dire che c'è un altro client
  //attualmente connesso con lo stesso username: la procedura fallisce mandando KO
  if(isInHashTable(HT,name)){
    sendKO(connfd,name,"REGISTER","Multiple clients with the same username", NULL);
    return;
  }

  //Setto l'utente corrente e lo inserisco nella tabella hash
  strcpy(currentUser,name);
  insertHashTable(&HT,currentUser);

  //Ottengo il path relativo all'utente
  char* user_path = getUserPath(name);

  //Provo a creare la direcytory
  int result = mkdir(user_path,  0755);
  free(user_path);

  //Se ho avuto successo nella creazione, mando OK
  if(result == 0)
    sendOK(connfd, currentUser, "REGISTER", NULL);
  else{
    //Se fallisco nella creazione, ma è perché la directory esiste già, va tutto bene
    //(l'utente si era già registrato in precedenza)
    if(errno == EEXIST)
      sendOK(connfd, currentUser, "REGISTER", NULL);
    //Se fallisco in ogni altro caso, vuol dire che ho avuto un problema e mando KO
    else
      sendKO(connfd, currentUser, "REGISTER", NULL, NULL);
  }

}

//Procedura relativa allo storing di un oggetto
void store(int connfd, char* currentUser ,char* name, long int len, char* newline){
  if(len < 0){
    sendKO(connfd, currentUser, "STORE", name, "Length not valid");
    return ;
  }


  //Creo il buffer che avrà il contenuto dell'oggetto (quindi di lunghezza len)
  void *data = calloc(len,1);

  //Con un semplice conto vado a calcolare la quantità di dati nella store che
  //ho già letto con la read nel thread worker
  int r = MAX_HEADER_SIZE - STORE_LENGTH - strlen(name) - getNumberOfDigits(len);

  //Se la differenza tra la lunghezza dei dati totali e la lunghezza dei dati letti è
  //maggiore di zero, allora vuol dire che non ho finito e devo fare una ulteriore read
  //di lunghezza len-r per finire la lettura totale
  if(len-r > 0){
    //Copio la porzione di dati dopo il newline (e lo spazio)
    memcpy(data,(newline+2),r);

    //Imposto il file descriptor come bloccante
    setBlockingFD(connfd, true);

    //Sfruttando l'aritmetica dei puntatori, eseguo una read, di dimensione len-r
    //(ovvero della stessa quantità dei dati rimanenti da leggere) e vado a copiarlo
    //in data (mantenendo la porzione precedente di dati)
    int n = readn(connfd, ((char*) data)+r,len-r);

    //Rimetto il file descriptor in modalità unblocking
    setBlockingFD(connfd, false);

    //Ho avuto un problema nel leggere la parte supplementare dei dati,
    //e quindi mando KO
    if(n <= 0){sendKO(connfd, currentUser, "STORE", name, NULL); return;}

  }
  //Se la differenza è uguale a zero (o minore) vuol dire che ho già letto
  //tutti i dati che mi servivano e posso procedere con la copia dei dati
  else memcpy(data,(void*)(newline+2),len);

  //Se per qualche motivo l'utente non è registrato, fallisco con KO
  if(currentUser[0] == '\0')
    sendKO(connfd, currentUser, "STORE", name, "User not registered");

  //L'utente è registrato (come normalmente dovrebbe essere)
  else{
      //Provo a creare il file oggetto con il contenuto letto, in caso di successo
      //mando OK, altrimenti KO
      if(createFile(name,data,currentUser,len))
        sendOK(connfd, currentUser, "STORE", name);
      else
        sendKO(connfd, currentUser, "STORE", name, NULL);
  }

  //Libero il buffer temporaneo dalla memoria
  free(data);
}

//Procedura relativa al retrieve dei dati
void retrieve(int connfd, char* currentUser, char* name){

  //Ottengo il path relativo all'oggetto
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,name);

  //Apro il file in lettura
  int new_fd = open(file_path, O_RDONLY, S_IRUSR | S_IWUSR);

  //Open utilizzata senza problemi
  if(new_fd != -1){
    //Tramite la funzione 'stat' ottengo la dimensione in byte del file
    struct stat finfo;
    stat(file_path, &finfo);
    size_t size = finfo.st_size;

    //Creo un blocco della stessa dimensione
    void *block = calloc(size, 1);

    //Metto il contenuto del file dentro il buffer del blocco
    readn(new_fd, block ,size);

    //Preparo la dimensione e il buffer per il messaggio da inviare
    //al client
    int N = DATA_MSG_LENGTH + getNumberOfDigits(size);
    char* buff = calloc(N + 1, sizeof(char));

    sprintf(buff,"DATA %lu \n ", size);

    //Prendo il contenuto del buffer temporaneo e, copiando anche il blocco
    //creo il buffer 'definitivo'
    char* def = calloc(N+1+size,sizeof(char));
    memcpy(def,buff,N);
    memcpy(def+N,block,size);

    //Invio il messaggio finito
    writen(connfd,def,N+size);

    //Stampo le info a schermo
    fprintf(target_output,"%-15s %-10s\tfd: %-10dop: %-10s\tOK \n",  currentUser, name, connfd, "RETRIEVE");

    //Libero le risorse
    free(buff);
    free(def);
    free(block);
    close(new_fd);
 	 }
   //In caso di errore invece mando KO
   else
    sendKO(connfd, currentUser, "RETRIEVE", name, "Can't retrieve the object");

  free(user_path);
  free(file_path);
}


//Procedura relativa alla eliminazione di un determinato oggetto
void delete(int connfd, char* currentUser, char* obj_name){
  //Ottengo il path relativo all'oggetto
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(obj_name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,obj_name);

  //Tento la rimozione
  bool success = (remove(file_path) == 0);

  //Se ho avuto successo mando OK, altrimenti KO
  if(success) sendOK(connfd, currentUser, "DELETE", obj_name);
  else sendKO(connfd, currentUser, "DELETE",obj_name , NULL);

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
