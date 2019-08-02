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

#include <hashtable.h>
#include <lib.h>

linkedlist *insertLinkedList(linkedlist *head, char* name){
  linkedlist *new = calloc(1,sizeof(linkedlist));
  new->name = strdup(name);
  /*new->name = calloc(strlen(name)+1,sizeof(char));
  strcpy(new->name,name);*/
  new->next = head;

  return new;
}

void printLinkedList(linkedlist *head){
  if(head == NULL){
    printf("NULL\n");
    return;
  }
  printf("%s -> ",head->name);
  printLinkedList(head->next);
}

bool isInLinkedList(linkedlist *head, char* target){
  if(head == NULL)return false;

  if(str_equals(target,head->name))
    return true;

  return isInLinkedList(head->next,target);
}

linkedlist *removeLinkedList(linkedlist* head, char* target){
  if(head == NULL)return NULL;

  linkedlist* curr = head;
  linkedlist* prec = NULL;

  while(curr != NULL){
    if(strcmp(curr->name,target) == 0){
      if(prec == NULL){
        head = curr->next;
        free(curr);
        return head;
      }
      else{
      prec->next = curr->next;
      free(curr);
      return head;
      }
    }

    prec = curr;
    curr = curr->next;
  }

 return NULL;
}


void freeLinkedList(linkedlist *head){
  linkedlist *curr = head, *tmp;

  while(curr != NULL){
    tmp = curr;
    curr = curr->next;
    free(tmp->name);
    free(tmp);
  }
  head = NULL;
}


unsigned long hash(char *str){
    unsigned long hash = 5381;
    int c = *str++;

    while(c){
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
      c = *str++;
    }

    return hash;
}

hashtable createHashTable(size_t N){
  hashtable T;
  //T.ht = calloc(size,sizeof(linkedlist*));
  T.field = calloc(N,sizeof(cell));
  T.size = N;
  return T;
}

hashtable insertHashTable(hashtable T, char* elem){
  unsigned long h = hash(elem) % T.size;
  pthread_mutex_lock(&(T.field[h].mtx));
  //DEBUG_PRINT("Insert: Entering mutex %lu\n",h);

  T.field[h].list = insertLinkedList(T.field[h].list, elem);

//  DEBUG_PRINT("Insert: Exiting mutex %lu\n",h);
  pthread_mutex_unlock(&(T.field[h].mtx));

  return T;
}

void printHashTable(hashtable T){
  for(int i = 0; i < T.size; i++){
    //DEBUG_PRINT("DEBUG_PRINT: Entering mutex %d\n",i);
    pthread_mutex_lock(&(T.field[i].mtx));
    printf("Cella [%d]: ",i);
    printLinkedList(T.field[i].list);

    pthread_mutex_unlock(&(T.field[i].mtx));
  //  DEBUG_PRINT("DEBUG_PRINT: Exiting mutex %d\n",i);
  }
}

bool isInHashTable(hashtable T, char* name){
  if(name == NULL)return false;
  unsigned long h = hash(name) % T.size;

  //DEBUG_PRINT("IsIn: Entering mutex %lu\n",h);
  pthread_mutex_lock(&(T.field[h].mtx));
  bool result = isInLinkedList(T.field[h].list,name);
  pthread_mutex_unlock(&(T.field[h].mtx));
  //DEBUG_PRINT("IsIn: Exiting mutex %lu\n",h);

  return result;
}

hashtable removeHashTable(hashtable T, char* target){
  if(target == NULL)return T;
  unsigned long h = hash(target) % T.size;

  pthread_mutex_lock(&(T.field[h].mtx));
  T.field[h].list = removeLinkedList(T.field[h].list,target);
  pthread_mutex_unlock(&(T.field[h].mtx));

  return T;
}

void freeHashTable(hashtable T){
  for(int i = 0; i < (T.size); i++){
  //  DEBUG_PRINT("Delete: Entering mutex %d\n",i);
    pthread_mutex_lock(&(T.field[i].mtx));
    freeLinkedList(T.field[i].list);
    pthread_mutex_unlock(&(T.field[i].mtx));
  //  DEBUG_PRINT("Delete: Exiting mutex %d\n",i);
  }

  free(T.field);
//  free(T);
}
