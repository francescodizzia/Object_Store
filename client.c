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

  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);


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
    os_store(argv[2],buffer, size);
    free(buffer);
    fclose(f);
    //createFile("test",buffer,"");

    read(fd,response_buf,MAX_RESPONSE_SIZE);
  }
  else if(str_equals(argv[3],"register")){
    os_connect(argv[1]);
    read(fd,response_buf,MAX_RESPONSE_SIZE);
  }

  printf("[pid: %d] ho ricevuto: %s\n",(int) getpid(),response_buf);

  close(fd);
  printf("\nCLOSED!");

  return 0;
}
