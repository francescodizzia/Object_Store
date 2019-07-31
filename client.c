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

bool terminate = false;


void sigIntHandler(){
 terminate = true;
 printf("ZA WARUDOOO\n");
}

void getResponseMsg(){
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);

  read(fd,response_buf,MAX_RESPONSE_SIZE);

  printf("I got: %s\n",response_buf);
}

void test1(){

  os_store("test0","xaxaxa", 6);
  getResponseMsg();
  os_connect("utente_prova");
  getResponseMsg();
  os_connect("utente_prova_2");
  getResponseMsg();
//  os_connect("utente_prova");
//  getResponseMsg();
  char b[] = "1234567890123456789012345678901234567890_1234567890123456789012345678901234567890_1";
  os_store("test1",b, strlen(b));
  getResponseMsg();
/*  os_store("test2","this one too", strlen("this one too"));
  getResponseMsg();
*/
}


int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);

  struct sockaddr_un serv_addr;

  int c;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);


  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);


  if( (c = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1){
    printf("[-] Server not up\n");
    return -1;
  }
/*
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);
*/
  test1();

/*
  if(str_equals(argv[3],"store")){
    FILE *f;


    f = fopen(argv[1], "rb");
    int d = fileno(f);

    struct stat finfo;
    fstat(d, &finfo);

    size_t size = finfo.st_size;

    char *buffer = calloc(size,1);
    if(f)
      fread(buffer, size, 1, f);

//    buffer[strlen(buffer)-1] = '\0';
    printf("\nBAKANA %ld\n",size);

  //  os_store(argv[1],argv[2], strlen(argv[2]));
  //  free(buffer);
  //  fclose(f);
    //createFile("test",buffer,"");

    read(fd,response_buf,MAX_RESPONSE_SIZE);
  }
  else if(str_equals(argv[3],"register")){
    os_connect(argv[1]);
    read(fd,response_buf,MAX_RESPONSE_SIZE);
  }

  printf("[pid: %d] ho ricevuto: %s\n",(int) getpid(),response_buf);
*/
  close(fd);
  printf("\nCLOSED!");

  return 0;
}
