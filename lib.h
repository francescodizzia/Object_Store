#define true 1
#define false 0
#define bool int

#define SOCKNAME "fradiz"
#define MAX_CONN 50
#define N 512
#define BYTES_TO_READ 4
#define MAX_HEADER_SIZE 512

#define ASSERT_NULL(a,e){\
  if(a == NULL){\
   perror(e);\
   printf("%s\n",e);\
   return false;\
  }\
}

bool str_equals(char* a, char* b){
 ASSERT_NULL(a,"a is NULL");
 ASSERT_NULL(b,"b is NULL");

 return (strcmp(a,b) == 0);
}

static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

  while(left > 0) {
	   if ((r = read((int)fd ,bufptr,left)) == -1) {
	     if (errno == EINTR)
        continue;

      return -1;
	   }

  if (r == 0)
   return 0;

  left -= r;
	bufptr += r;
  }

  return size;
}


static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

  while(left > 0) {
	 if((r = write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	 }

   if(r == 0)
    return 0;

   left -= r;
	 bufptr += r;
  }

    return 1;
}
