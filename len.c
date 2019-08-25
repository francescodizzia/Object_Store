#include <stdio.h>
#include <stdlib.h>

#include <lib.h>

int main(){
 os_connect("prova");
 os_store("oggetto", NULL, 0);
 os_disconnect();


return 0;
}
