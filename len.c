#include <stdio.h>
#include <stdlib.h>

#include <lib.h>

int main(){
 os_connect("prova");
 os_store("oggetto", "a1234567", 8);
 void* a = os_retrieve("oggetto");
 os_store("oggetto_copy", a, 8);

 return 0;
}
