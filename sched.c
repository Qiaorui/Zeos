/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

//#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
//#endif

extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct *idle_task;
int new_pid;
#define DEFAULT_QUANTUM 100
int remaining_quantum=0;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


void update_stats(unsigned long *v, unsigned long *elapsed)
{
  unsigned long current_ticks;
  
  current_ticks=get_ticks();
  
  *v += current_ticks - *elapsed;
  
  *elapsed=current_ticks;
  
}

void init_stats(struct stats *s)
{
	s->user_ticks = 0;
	s->system_ticks = 0;
	s->blocked_ticks = 0;
	s->ready_ticks = 0;
	s->elapsed_total_ticks = get_ticks();
	s->total_trans = 0;
	s->remaining_ticks = get_ticks();
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

int getNewPid(){
  ++new_pid;
  return new_pid;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
        printk("CPU IDLE");
	while(1)
	{
	;
	}
}

void init_idle (void)
{
  struct task_struct* tmp = list_head_to_task_struct(list_first(&freequeue));
  list_del(list_first(&freequeue));
  tmp->PID = 0;
  allocate_DIR(tmp);
  tmp->quantum = DEFAULT_QUANTUM;
  //cpu_idle();
  idle_task = tmp;
  init_stats(&tmp->stats);


  union task_union* tmp_task = (union task_union*)tmp;
  tmp_task->stack[1023] = (int)cpu_idle;
  tmp_task->stack[1022] = 0;
  tmp_task->task.kernel_esp = (int)&tmp_task->stack[1022];

}

void init_task1(void)
{

  struct task_struct* tmp = list_head_to_task_struct(list_first(&freequeue));
  list_del(list_first(&freequeue));
  tmp->PID = 1;
  tmp->state = ST_RUN;
  tmp->quantum = DEFAULT_QUANTUM;
  remaining_quantum = DEFAULT_QUANTUM;
  init_stats(&tmp->stats);

  allocate_DIR(tmp);
  //cpu_idle();
  set_user_pages(tmp);
//UPDATE TSS
  union task_union* tmp_task = (union task_union*)tmp;
  tss.esp0 = (DWord)&tmp_task->stack[KERNEL_STACK_SIZE];
  set_cr3(get_DIR(tmp));  
}


void init_sched(){

  //Initialization of freequeue
  INIT_LIST_HEAD(&freequeue);
  int i=0;
  while (i<NR_TASKS) {
    list_add_tail(&task[i].task.list, &freequeue);
    ++i;
  }

  //Initialization of readyqueue
  INIT_LIST_HEAD(&readyqueue);

  new_pid = 1;
  

}

void inner_task_switch(union task_union* new){
  struct task_struct* current_task_struct = current();
  struct task_struct* new_task = (struct task_struct *)new;
  tss.esp0 = (DWord)&new->stack[KERNEL_STACK_SIZE];

  set_cr3(get_DIR(new_task));

  __asm__ __volatile__("movl %%ebp, %0":"=r"(current_task_struct->kernel_esp));

  __asm__ __volatile__("movl %0, %%esp"::"r"(new_task->kernel_esp));

  __asm__ __volatile__("popl %ebp \n"
                       "ret \n");
}


void task_switch(union task_union* new){
  __asm__ __volatile__ (//"pushl %%eax\n"
          //Must save EBX ESI EDI
    "pushl %esi\n"
    "pushl %ebx\n"
    "pushl %edi\n"
   );
  
  inner_task_switch(new);

  __asm__ __volatile__ (//"pushl %%eax\n"
          //Must save EBX ESI EDI
    "popl %edi\n"
    "popl %ebx\n"
    "popl %esi\n"
   );
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}




int get_quantum(struct task_struct *t)
{
  return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
  t->quantum=new_quantum;
}

void sched_next_rr(){
  struct list_head *e;
  struct task_struct* next;


  if (!list_empty(&readyqueue))
  {

    e=list_first(&readyqueue);
    next = list_head_to_task_struct(e);
    list_del(e);
  }
  else
    next=idle_task;

  next->state=ST_RUN;
  remaining_quantum=get_quantum(next);

  update_stats(&(current()->stats.system_ticks), &(current()->stats.elapsed_total_ticks));
  update_stats(&(next->stats.ready_ticks), &(next->stats.elapsed_total_ticks));
  next->stats.total_trans++;

  task_switch((union task_union*)next);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest){
  if (t->state!=ST_RUN) list_del(&(t->list));
  if (dest!=NULL)
  {
    list_add_tail(&(t->list), dest);
    if (dest!=&readyqueue) t->state=ST_BLOCKED;
    else
    {
      update_stats(&(current()->stats.system_ticks), &(current()->stats.elapsed_total_ticks));
      t->state=ST_READY;
    }
  }
  else t->state=ST_RUN;
}


int needs_sched_rr() {
  if ((remaining_quantum==0)&&(!list_empty(&readyqueue))) return 1;
  if (remaining_quantum==0) remaining_quantum=get_quantum(current());
  return 0;
}


void update_sched_data_rr(){
  --remaining_quantum;
}

void schedule() {
  update_sched_data_rr();
  if (needs_sched_rr())
  {
    update_process_state_rr(current(), &readyqueue);
    sched_next_rr();
  }
}








