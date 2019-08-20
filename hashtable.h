#if !defined(_HASHTABLE)
#define _HASHTABLE
#include <pthread.h>

//Struttura per le linked list
typedef struct _linkedlist {
  char* name;
  struct _linkedlist *next;
} linkedlist;

//Struttura per le celle della hash table
typedef struct _cell {
  linkedlist* list;
  pthread_mutex_t mtx;
} cell;

//Struttura della hash table
typedef struct _hashtable{
  cell *field;
  size_t size;
} hashtable;

void insertLinkedList(linkedlist* *head, char* name);
void printLinkedList(linkedlist *head);
bool isInLinkedList(linkedlist *head, char* target);
void removeLinkedList(linkedlist* *head, char* target);
void freeLinkedList(linkedlist* *head);

unsigned long hash(char *str);
hashtable createHashTable(size_t N);
void insertHashTable(hashtable *T, char* elem);
void printHashTable(hashtable T);
bool isInHashTable(hashtable HT, char* name);
void removeHashTable(hashtable *HT, char* target);
void freeHashTable(hashtable *T);

#endif
