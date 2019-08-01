#include <lib.h>

#if !defined(_HASH)
#define _HASH

typedef struct _linkedlist {
  char* name;
  struct _linkedlist *next;
} linkedlist;

typedef struct _hashtable{
  linkedlist** ht;
  size_t size;
} hashtable;

linkedlist *insertLinkedList(linkedlist *head, char* name);
void printLinkedList(linkedlist *head);
bool isInLinkedList(linkedlist *head, char* target);
void freeLinkedList(linkedlist* head);

unsigned long hash(char *str);
hashtable createHashTable(size_t N);
hashtable insertHashTable(hashtable T, char* elem);
void printHashTable(hashtable T);
bool isInHashTable(hashtable HT, char* name);
void deleteHashTable(hashtable T);

#endif
