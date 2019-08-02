#include <lib.h>
#include <pthread.h>

#if !defined(_HASHTABLE)
#define _HASHTABLE


typedef struct _linkedlist {
  char* name;
  struct _linkedlist *next;
} linkedlist;

typedef struct _cell {
  linkedlist* list;
  pthread_mutex_t mtx;
} cell;

typedef struct _hashtable{
  cell *field;
  size_t size;
} hashtable;

linkedlist *insertLinkedList(linkedlist *head, char* name);
void printLinkedList(linkedlist *head);
bool isInLinkedList(linkedlist *head, char* target);
linkedlist *removeLinkedList(linkedlist* head, char* target);
void freeLinkedList(linkedlist* head);

unsigned long hash(char *str);
hashtable createHashTable(size_t N);
hashtable insertHashTable(hashtable T, char* elem);
void printHashTable(hashtable T);
bool isInHashTable(hashtable HT, char* name);
hashtable removeHashTable(hashtable HT, char* target);
void freeHashTable(hashtable T);

#endif
