#include <libc.h>

char buff[24];

int pid;

void coutStr(char* msg){
  int size = strlen(msg);
  write(1,msg,size);
}

void coutInt(int n) {
  char msg[10];
  itoa(n,msg);
  int size = strlen(msg);
  write(1,msg,size);
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    //__asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); 
  write (1,"\n\n\n\n",4);

  //SYS_WRITE TEST
  coutStr("[sys_write] ok\n");

  //CLOCK TEST
  int clock_t=gettime();
  int volatile a =0;
  while(a<3) {
  while(gettime() == clock_t) {
  }
  clock_t=gettime();
  coutStr("[clock] ");
  coutInt(gettime());
  a=a+1;
  coutStr("  ok\n"); 
 }


  //errno TEST
  if (write(0,"a",1)< 0)
  perror("[ERRNO TEST]Should be a error message");  

  while(1) { }
}
