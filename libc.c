/*
 * libc.c 
 */

#include <libc.h>

#include <errno.h>
#include <types.h>

int errno;



void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write(int fd, char *buffer, int size) {
  int result=-1;
  __asm__(//"pushl %%eax\n"
          //Must save EBX ESI EDI
	  "pushl %%ebx\n"
	  "movl %1, %%ebx\n"
	  "movl %2, %%ecx\n"
	  "movl %3, %%edx\n"
	  "movl $4, %%eax\n"
	  "int $0x80\n"
	  "movl %%eax, %0\n" 
	  "popl %%ebx\n"
	  //"popl %%eax"
	  :"=r"(result)
	  :"m"(fd) ,"m"(buffer) ,"m"(size) 
	 );
  if (result < 0) {
    errno = result;
    return -1;
  }
  return result;
}


//TMP
void cout(char* msg){
  int size = strlen(msg);
  write(1,msg,size);
}



void perror(char* msg) {
  cout(msg);

  switch(errno){
    case EBADF:
      cout(": Bad file number");
      break;
    case EACCES:
      cout(": Permission denied");
      break;
    case EINVAL:
      cout(": Invalid argument");
      break;
    case ENOSYS:
      cout(": Function not implemented");
      break;
    default:
      cout("Unknown error");
  }
}


int gettime() {
  int result=-1;
  __asm__(//"pushl %%eax\n"
	  "movl $10, %%eax\n"
	  "int $0x80\n"
	  "movl %%eax, %0\n" 
	  //"popl %%eax"
	  :"=r"(result)
	 );
  if (result < 0) {
    errno = result;
    return -1;
  }
  return result;
}

