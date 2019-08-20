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

#include <hashtable.h>
#include <common.h>

//Inserisce la stringa 'name' nella lista con testa 'head'
void insertLinkedList(linkedlist* *head, char* name){
  linkedlist *new = calloc(1, sizeof(linkedlist));
  new->name = strdup(name);
  new->next = *head;

  *head = new;
}

//Stampa la lista (utile per il debug)
void printLinkedList(linkedlist *head){
  if(head == NULL){
    printf("NULL\n");
    return;
  }
  printf("%s -> ",head->name);
  printLinkedList(head->next);
}

//Verifica se l'elemento è presente nella lista o meno
//restituendo rispettivamente TRUE o FALSE
bool isInLinkedList(linkedlist *head, char* target){
  if(head == NULL)return false;

  if(str_equals(target,head->name))
    return true;

  return isInLinkedList(head->next,target);
}

//Rimuove l'elemento dalla lista
void removeLinkedList(linkedlist* *head, char* target){
  if(*head == NULL)return;

  linkedlist* curr = *head;
  linkedlist* prec = NULL;

  while(curr != NULL){
    if(strcmp(curr->name,target) == 0){
      if(prec == NULL){
        *head = curr->next;
        free(curr->name);
        curr->name = NULL;
        free(curr);
        curr = NULL;
        return;
      }
      else{
      prec->next = curr->next;
      free(curr->name);
      curr->name = NULL;
      free(curr);
      curr = NULL;
      return;
      }
    }

    prec = curr;
    curr = curr->next;
  }

}

//Esegue la free su tutti gli elementi della lista
void freeLinkedList(linkedlist* *head){
  linkedlist *curr = *head, *tmp;

  while(curr != NULL){
    tmp = curr;
    curr = curr->next;
    free(tmp->name);
    tmp->name = NULL;
    free(tmp);
    tmp = NULL;
  }
  head = NULL;
}

//Funzione Hash 'djb2', di Dan Bernstein (http://www.cse.yorku.ca/~oz/hash.html)
unsigned long hash(char *str){
    unsigned long hash = 5381;
    int c = *str++;

    while(c){
      hash = ((hash << 5) + hash) + c;
      c = *str++;
    }

    return hash;
}

//Crea e alloca l'hash table
hashtable createHashTable(size_t N){
  hashtable T;
  T.field = calloc(N,sizeof(cell));
  T.size = N;
  return T;
}

//Inserisce l'elemento nella tabella hash
void insertHashTable(hashtable *T, char* elem){
  unsigned long h = hash(elem) % T->size;
  pthread_mutex_lock(&(T->field[h].mtx));
  insertLinkedList(&(T->field[h].list), elem);
  pthread_mutex_unlock(&(T->field[h].mtx));

}

//Stampa tutta la tabella hash (utile per il debug)
void printHashTable(hashtable T){
  for(int i = 0; i < T.size; i++){
    pthread_mutex_lock(&(T.field[i].mtx));
      printf("Cella [%d]: ",i);
      printLinkedList(T.field[i].list);
    pthread_mutex_unlock(&(T.field[i].mtx));
  }
}

//Verifica se un elemento è presente o meno nella tabella
bool isInHashTable(hashtable T, char* name){
  if(name == NULL)return false;
  unsigned long h = hash(name) % T.size;

  pthread_mutex_lock(&(T.field[h].mtx));
  bool result = isInLinkedList(T.field[h].list,name);
  pthread_mutex_unlock(&(T.field[h].mtx));
  return result;
}


//Rimuove l'elemento dalla tabella hash
void removeHashTable(hashtable *T, char* target){
  if(target == NULL)return;
  unsigned long h = hash(target) % T->size;

  pthread_mutex_lock(&(T->field[h].mtx));
  removeLinkedList(&(T->field[h].list),target);
  pthread_mutex_unlock(&(T->field[h].mtx));

}

//Esegue la free di tutti gli elementi della hash table
void freeHashTable(hashtable *T){
  for(int i = 0; i < (T->size); i++){
    pthread_mutex_lock(&(T->field[i].mtx));
      freeLinkedList(&(T->field[i].list));
    pthread_mutex_unlock(&(T->field[i].mtx));
  }

  free(T->field);
}
