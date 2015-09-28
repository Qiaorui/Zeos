/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>
#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

int sys_write(int fd, char* buffer, int size) {
  int ret = check_fd(fd, ESCRIPTURA);
  if (ret != 0) return ret;
  if (buffer == NULL || size < 0 || fd != 1) return EINVAL;
  char kernel_data[size];
  ret = copy_from_user(buffer, kernel_data, size);
  if (ret != 0) return ret;
  sys_write_console(kernel_data, size);
  return 0;
}

int sys_gettime(){
  int result = sys_gettime_console();
  
  return result;
}

