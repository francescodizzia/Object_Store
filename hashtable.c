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

hashtable createHashTable(size_t size){
  hashtable T;
  T.ht = calloc(size,sizeof(linkedlist*));
  T.size = size;
  return T;
}

hashtable insertHashTable(hashtable T, char* elem){
  unsigned long h = hash(elem) % T.size;
  T.ht[h] = insertLinkedList(T.ht[h], elem);
  return T;
}

void printHashTable(hashtable T){
  for(int i = 0; i < T.size; i++){
    printf("Cella [%d]: ",i);
    printLinkedList(T.ht[i]);
  }
}

bool isInHashTable(hashtable T, char* name){
  if(name == NULL)return false;
  unsigned long h = hash(name) % T.size;
  return isInLinkedList(T.ht[h],name);
}

void deleteHashTable(hashtable T){
  for(int i = 0; i < (T.size); i++)
    freeLinkedList(T.ht[i]);

  free(T.ht);
//  free(T);
}
