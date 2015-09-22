#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    //__asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); 

  //int $0x80;
  write (1,"\n\n\n\n",4);
  char* msg = "[sys_write] ok\n";
  int size = strlen(msg);
  write(1,msg,size);
  if (gettime() != 0) write(1,"clock:>",7);
  else write(1,"clock:0",7);


  while(1) { }
}
