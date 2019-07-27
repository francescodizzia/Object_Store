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

 char buf[MAX_HEADER_SIZE];
 memset(buf, '\0', MAX_HEADER_SIZE);

 char nome[34];
 memset(nome, '\0', 34);

 strcpy(nome,argv[1]);
 strcpy(buf,argv[2]);


  while(!terminate){
    //memset(buf, '\0', MAX_HEADER_SIZE);
   //if(argv[2] == NULL)
    //fgets(buf, MAX_HEADER_SIZE, stdin);

   if(str_equals(argv[3],"store")){
    os_store(nome,buf, strlen(buf));
    readn(fd,buf,strlen(buf));
   }
   else if(str_equals(argv[3],"register")){
    os_connect(nome);
    memset(buf, '\0', MAX_HEADER_SIZE);
    readn(fd,buf,4);
   }

  pid_t pid = getpid();

  printf("[pid: %d] ho ricevuto: %s\n",(int) pid,buf);

    //if(str_equals(buf, "quit"))
     terminate = true;
  }


  close(fd);

  return 0;
}
