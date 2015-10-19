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

extern int remaining_quantum;

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

int ret_from_fork()
{
  return 0;
}

int sys_fork()
{
  // creates the child process

  if (list_empty(&freequeue)) return ENOSPC;
  struct task_struct  *new_task = list_head_to_task_struct(list_first(&freequeue));
  list_del(list_first(&freequeue));

  copy_data(current(), new_task, sizeof(union task_union));
  
  allocate_DIR(new_task);

  int data[NUM_PAG_DATA];
  int i, j;
  for (i = 0; i < NUM_PAG_DATA; ++i)
  {
    data[i] = alloc_frame();
    if (data[i] < 0)
    {
      for (j = 0; j <= i; ++j)
      {
        free_frame(data[j]);
      }
      list_add_tail(&new_task->list, &freequeue);
      return ENOMEM;
    }
  }



  page_table_entry* current_pt = get_PT(current());
  page_table_entry* new_pt = get_PT(new_task);

  
  for (i = 0; i < NUM_PAG_DATA; ++i)
  {
    set_ss_pag(new_pt,PAG_LOG_INIT_DATA+i,data[i]);
  }

  for (i = 0; i < NUM_PAG_KERNEL; ++i)
  {
    set_ss_pag(new_pt, i, get_frame(current_pt, i));
  }

  for (i=0; i<NUM_PAG_CODE; ++i)
  {
    set_ss_pag(new_pt, PAG_LOG_INIT_CODE+i, get_frame(current_pt, PAG_LOG_INIT_CODE+i));
  }

  for (i=NUM_PAG_KERNEL+NUM_PAG_CODE; i<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; i++)
  {
  
    set_ss_pag(current_pt, i+NUM_PAG_DATA, get_frame(new_pt, i));
    copy_data((void*)(i<<12), (void*)((i+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(current_pt, i+NUM_PAG_DATA);
  }
  

  set_cr3(get_DIR(current()));

  new_task->PID = getNewPid();

  union task_union* uchild = (union task_union*)new_task;
  

  int register_ebp;		

  __asm__ __volatile__ (
    "movl %%ebp, %0\n\t"
      : "=g" (register_ebp)
      : );
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.kernel_esp=register_ebp + sizeof(DWord);
  
  DWord temp_ebp=*(DWord*)register_ebp;
 
  uchild->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.kernel_esp)=(DWord)&ret_from_fork;
  uchild->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.kernel_esp)=temp_ebp;

  list_add_tail(&(uchild->task.list), &readyqueue);


  return uchild->task.PID;


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

