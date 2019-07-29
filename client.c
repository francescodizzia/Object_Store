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
  /*  char *file_contents;
    long input_file_size;
    FILE *input_file = fopen(input_file_name, argv[2]);
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    rewind(input_file);
    file_contents = malloc(input_file_size * (sizeof(char)));
    fread(file_contents, sizeof(char), input_file_size, input_file);
    fclose(input_file);
*/

    os_store(argv[1],argv[2], strlen(argv[2]));
    readn(fd,response_buf,MAX_RESPONSE_SIZE);
  }
  else if(str_equals(argv[3],"register")){
    os_connect(argv[1]);
    readn(fd,response_buf,MAX_RESPONSE_SIZE);
  }

  printf("[pid: %d] ho ricevuto: %s\n",(int) getpid(),response_buf);

  close(fd);
  printf("\nCLOSED!");

  return 0;
}
