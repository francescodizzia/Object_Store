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
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>

#include <lib.h>

#define _POSIX_C_SOURCE 200809L
#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

#define DEBUG_ENABLED 0

char* OK_RESPONSE = "OK \n";



#define SYSCALL(r,c,e) \
    if((r=c)==-1) { perror(e);printf(" errore:%d\n",errno); exit(1);}

#define DEBUG_CMD(c) \
  if(DEBUG_ENABLED) {c;}

static volatile sig_atomic_t running = true;
static pthread_mutex_t mtx;
static pthread_cond_t empty;

static int n_clients = 0;


void cleanup() {
  unlink(SOCKNAME);
}

void sigIntHandler(){
 running = false;
 printf("ZA WARUDOOO\n");
}

void toup(char *str) {
    char *p = str;
    while(*p != '\0') {
        *p = (islower(*p)?toupper(*p):*p);
	      ++p;
    }
}

void parse_request(int c_fd, char *str){
 if(str == NULL)return;

 int u;
 char *ptr = NULL;
 int len = -1;

 char *action = strtok_r(str, " ", &ptr);
 char *name = strtok_r(NULL, " ", &ptr);
 //char* len_s = strtok_r(NULL, " ", &ptr);
// size_t len = strtok_r(NULL, " ", &ptr);
//size_t len = atol(lenstr);
//len = (int) atol(len_s);
 //DEBUG_CMD(printf("ACTION: %s   NAME: %s LEN: %d\n", action, name, len));
 //printf("User: _%s_  s: %d\n",name,len);
 if(action == NULL)return ;

/*
 if(str_equals(action,"STORE")){
  DEBUG_CMD(printf("STORE\n"));
  char* buf = calloc(len+1, sizeof(char));
  SYSCALL(u,readn(c_fd, buf, len),"read_store");

  buf[len] = '\0';

  DEBUG_CMD(printf("data: %s\n", buf));
  toup(buf);
  SYSCALL(u,writen(c_fd,buf,len),"write_store");
  free(buf);
 }
 else
*/
  if(str_equals(action,"REGISTER") && name != NULL){
  DEBUG_CMD(printf("REGISTER\n"));

  chdir(DATA_DIRECTORY);
  int result = mkdir(name,  0755);

 if(result == 0){ //SUCCESSO
  writen(c_fd,"OK \n",MAX_RESPONSE_SIZE);
  printf("Invio OK\n");
 }
 else{
   printf("Invio FALLIMENTO\n");
   char fail_buf[MAX_RESPONSE_SIZE];
   memset(fail_buf,'\0',MAX_RESPONSE_SIZE);

   sprintf(fail_buf,"KO %d \n", errno);
   writen(c_fd,fail_buf,MAX_RESPONSE_SIZE);

 }
 printf("[%ld] *fine richiesta*\n\n",c_fd);
}

}

void *threadF(void *arg) {
  long connfd = (long)arg;

  char header[MAX_HEADER_SIZE*2];
   memset(header, '\0', MAX_HEADER_SIZE*2);

  char finalheader[512];
  memset(finalheader, '\0', 512);


  pthread_mutex_lock(&mtx);
  n_clients++;
  pthread_mutex_unlock(&mtx);

  while(running){
/*
   FD_ZERO(&readfds);
   FD_SET(connfd, &readfds);

   timeout.tv_sec = 0;
   timeout.tv_usec = 500000;

   sret = select(connfd+1,&readfds, NULL, NULL, NULL);

   if(sret == 0){
     printf("sret = %d\n", sret);
     printf(" timeout\n")
     ;
   }
   else{
     */
   int u=0;


  // pthread_cond_wait(&awaken,&printThread);
    SYSCALL(u,readn(connfd, header, DEFAULT_CHUNK_SIZE),"read_x");

   if(u == 0){  //pthread_mutex_unlock(&printThread);
break;}

   printf("\n[%ld]%s",connfd, header);

  //FAI COSE COL DATO RICEVUTO
	//toup(header);
   strcat(finalheader,header);



  if(header[u-1] == '\0'){
      parse_request(connfd, finalheader);
/*
      writen(connfd,"OK \n",MAX_RESPONSE_SIZE);
      printf("Invio OK\n");
  printf("[%ld] *fine richiesta*\n\n",connfd);
*/
  /*
  chdir("./users/");
  mkdir(finalheader,  0755);
  */
  memset(finalheader,'\0',512);
  }

//  pthread_mutex_unlock(&printThread);

	//writen(connfd, header, MAX_HEADER_SIZE);
//  }
}
  close(connfd);

  pthread_mutex_lock(&mtx);
  n_clients--;
  if(n_clients <= 0)
    pthread_cond_signal(&empty);

  pthread_mutex_unlock(&mtx);

  return NULL;
}

void spawn_thread(long connfd) {
    pthread_attr_t thattr;
    pthread_t thid;

    if(pthread_attr_init(&thattr) != 0) {
	    fprintf(stderr, "pthread_attr_init FALLITA\n");
	    close(connfd);
	    return;
    }

    // settiamo il thread in modalità detached
    if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
	   fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
     close(connfd);
     return;
    }

    if (pthread_create(&thid, &thattr, threadF, (void*)connfd) != 0) {
     fprintf(stderr, "pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   close(connfd);
	   return;
    }
}

int main(){
  cleanup();
  atexit(cleanup);

 //Ignoro SIGPIPE
 signal(SIGPIPE, SIG_IGN);
 signal(SIGINT, sigIntHandler);

 int fd = socket(AF_UNIX, SOCK_STREAM, 0);

 struct sockaddr_un sa;
 strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
 sa.sun_family = AF_UNIX;

 bind(fd, (struct sockaddr *)&sa, sizeof(sa));
 listen(fd, MAX_CONN);

 int sret,connfd;

 fd_set readfds;
 struct timeval timeout;

 int clients = n_clients;

 while(running){

   pthread_mutex_lock(&mtx);
    if(clients != n_clients)
     DEBUG_CMD(printf("%d\n",n_clients));
   clients = n_clients;
   pthread_mutex_unlock(&mtx);

   FD_ZERO(&readfds);
   FD_SET(fd, &readfds);

   timeout.tv_sec = 0;
   timeout.tv_usec = 100000;

   sret = select(fd+1,&readfds, NULL, NULL, &timeout);

   if(sret == 0){
    // printf("sret = %d\n", sret);
    // printf(" timeout\n")
    ;
   }
   else{
      if(sret == -1){
       printf("OMAE WA MO SHINDEIRU\n");
       break;
      }

      connfd = accept(fd, (struct sockaddr*)NULL ,NULL);
      spawn_thread(connfd);
    }



}


  pthread_mutex_lock(&mtx);
  if(n_clients > 0){
   printf("[X] WAITING FOR THE THREADS\n");
   pthread_cond_wait(&empty, &mtx);
   printf("[+] DONE\n");
  }
  pthread_mutex_unlock(&mtx);


 printf("[+] Tutti i thread sono stati terminati con successo!\n");

 //close(conn_fd);
 close(fd);


 return 0;
}
