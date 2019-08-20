#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <lib.h>
#include <stdbool.h>

#include <common.h>

//Macro per la verifica dei booleani
#define ASSERT_BOOL(c){\
  if(!c){\
    return false;\
  }\
}

//Si occupa di creare un array di dimensione N, in cui sono presenti in successione
//dei caratteri che vanno da '0' a '9'
char* makeTestArray(int N){
  char *A = calloc(N, 1);

    int j = 0;
    for(int i = 0; i < N; i++, j++){
        //Non voglio andare oltre la cifra '9', torno all'inizio
        if(j > 9) j = 0;
        //In funzione del parametro j (che va da 0 a 9) riempio l'elemento
        //i dell'array A
        A[i] = '0' + j;
    }

  //Restituisco l'array
  return A;
}


//Eseguo il test 1: store di 20 oggetti di dimensioni diverse
bool test1(char* user){
  char obj_name[128];
  memset(obj_name,'\0',128);

  int size = 0;
  char* test;

  //Tento la connessione al server
  ASSERT_BOOL(os_connect(user));

  //Creo i 20 oggetti di dimensione crescente, a partire da 100 byte, per poi arrivare
  //a 100 kilobyte
  for(int i = 0; i < 20; i++){
    //Funzione che descrive la crescita della dimensione dei vari oggetti
    //Parte da 100, perché durante la prima chiamata i sarà uguale a zero
    size = 100 + ((i*17)*(i*17));

    //Dato che la funzione durante l'ultima chiamata va leggermente a sforare
    //i 100kb, vado a correggere manualmente il tutto
    if(size > 100000)
      size = 100000;

    //Imposto il formato dei nomi degli oggetti
    //(il primo oggetto sarà object_1, il secondo object_2, e così via...)
    sprintf(obj_name,"object_%d",i+1);

    //Creo l'array di test grande 'size' byte
    test = makeTestArray(size);

    //Provo ad eseguire la store del suddetto oggetto
    ASSERT_BOOL(os_store(obj_name, test, size));

    //Posso liberare la memoria
    free(test);
  }

  //Tento la disconnessione
  ASSERT_BOOL(os_disconnect());


  //Se sono arrivato fino a qui, vuol dire che ho concluso con successo
  return true;
}


//Test di tipo 2: esecuzione del retrieve dei 20 oggetti e conseguente verifica
//del contenuto
bool test2(char* user){

  //Provo a connettermi al server
  ASSERT_BOOL(os_connect(user));

  //Inizializzo il buffer su cui andrà l'oggetto ottenuto dalla retrieve
  void *retrieved_obj = NULL;
  char obj_name[128];
  memset(obj_name, '\0', 128);

  int size = 0;
  char *test;

  //Per tutti i 20 oggetti eseguo la retrieve e verifico che il contenuto degli
  //oggetti sia corretto
  for(int i = 0; i < 20; i++){
    //Allo stesso modo del test 1 procedo con l'ottenere il formato del nome degli
    //oggetti e la corrispettiva dimensione
    sprintf(obj_name,"object_%d",i+1);
    size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    //Eseguo la retrieve dell'oggetto dato
    retrieved_obj = os_retrieve(obj_name);

    //La retrieve restituisce NULL (è dunque fallita)
    if(retrieved_obj == NULL){
      return false;
    }

    //Creo l'array test della dimensione del rispettivo oggetto di cui
    //ho effettuato la retrieve
    test = makeTestArray(size);

    //Attraverso la funzione memcmp eseguo un confronto di 'size' byte
    //tra il buffer contenente il contenuto della retrieve e l'array di test
    //Se, come nella strcmp, viene ritornato un valore diverso da zero vuol dire
    //che i contenuti sono diversi: qualcosa molto probabilmente è andato storto
    //o nella retrieve o nella store del'oggetto
    if(memcmp(retrieved_obj, test, size) != 0){
      printf("ERROR: contents are not equal\n");
      return false;
    }

    memset(obj_name, '\0', 128);

    //Libero i buffer che non mi servono più
    free(retrieved_obj);
    free(test);
  }

  //Provo ad effettuare la disconessione dal server
  ASSERT_BOOL(os_disconnect());


  //Tutto è andato liscio, ritorno TRUE
  return true;
}


//Test di tipo 3: eliminazione dei 20 oggetti
bool test3(char* user){
  char obj_name[128];
  memset(obj_name, '\0', 128);

  //Provo a connettermi
  ASSERT_BOOL(os_connect(user));

  //Richiedo la cancellazione dei 20 oggetti
  for(int i = 0; i < 20; i++){
    //Ottengo, come fatto in precedenza, il nome dell'oggetto su cui
    //dovrò operare
    sprintf(obj_name,"object_%d",i+1);

    //Tento di eliminare l'oggetto con nome 'obj_name' (calcolato sopra)
    ASSERT_BOOL(os_delete(obj_name));
    memset(obj_name, '\0', 128);
  }

  //Provo a disconettermi
  ASSERT_BOOL(os_disconnect());


  //Nessun errore: ho concluso e posso ritornare TRUE
  return true;
}


//Test di tipo 4: un piccolo test extra per dimostrare che l'ObjectStore
//è in grado di agire anche su file di tipo binario, e non solo con stringhe
//(come fatto vedere con i test 1 e 2)
bool test4(char* user, char* src, char* dest){
  bool result = false;

  //Tento la connessione
  ASSERT_BOOL(os_connect(user));

  //Apro il file con il path sorgente (src) in modalità lettura
  int new_fd = open(src, O_RDONLY, S_IRUSR | S_IWUSR);

  //In caso di errore termino ritornando FALSE
  if(new_fd == -1) return false;

  //Uso la funzione stat per ottenere la grandezza in byte del file
  struct stat finfo;
  if(stat(src, &finfo) == -1)return false;

  size_t size = finfo.st_size;

  //Creo un buffer della stessa grandezza che userò per immagazzinare
  //il contenuto del file
  char *buffer = calloc(size, sizeof(char));
  if(!buffer) return false;

  //Leggo il contenuto del file e lo metto nel buffer
  readn(new_fd, buffer, size);

  //Effettuo la store, inviando come dati il buffer manipolato poco prima
  result = os_store(dest, buffer, size);

  //Libero la memoria e chiudo il fd
  free(buffer);
  close(new_fd);

  //Provo a disconnettermi
  ASSERT_BOOL(os_disconnect());

  //A questo punto l'esito dipende dell'operazione dipende dalla store,
  //ritorno quindi l'esito della store
  return result;
}


int main(int argc,char* argv[]){

  //L'utente ha avviato il client specificando meno argomenti del dovuto
  if(argc < 3){
    printf("Riprovare col giusto numero di argomenti\n");
    return -1;
  }

  //Variabile booleana che sarà valutata per verificare l'esito dei test
  bool result = false;

  //Ottengo la stringa relativa all'utente e converto in un intero
  //il codice che descrive il tipo di test
  char* user = strdup(argv[1]);
  int test_code = atol(argv[2]);

  //Eventuali argomenti extra, utilizzabili per eseguire il test 4
  //(piccolissimo test extra, vedere la relazione per maggiori dettagli)
  char* data_src = NULL;
  char* dest = NULL;

  //Li considero solo se ci sono almeno 4 argomenti
  if(argc > 3){
  data_src = strdup(argv[3]);
  dest = strdup(argv[4]);
  }


  //In base al numero specificato da shell, vado a effettuare il relativo test
  if(test_code == 1)
    result = test1(user);
  else if(test_code == 2)
    result = test2(user);
  else if(test_code == 3)
    result = test3(user);
  else if(test_code == 4 && data_src && dest)
    result = test4(user, data_src, dest);

  //Esito positivo: stampo un messaggio di successo
  if(result)
    printf("[OK] Test %d passato con successo! [utente: %s]\n", test_code, user);
  else{
    //Esito negativo: stampo un messaggio di fallimento, con un messaggio d'errore
    //che spiega il motivo del fallimento (vedere in 'lib.c' printLastErrorMsg() e come
    //viene settato l'errore nelle varie funzioni)
    printf("[KO] Test %d fallito... [utente: %s]\n", test_code, user);
    printLastErrorMsg();
  }

  //Libero la memoria e termino
  free(user);
  if(data_src) free(data_src);
  if(dest) free(dest);
  return 0;
}
