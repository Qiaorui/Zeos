#ifndef DEVICES_H__
#define  DEVICES_H__

extern int zeos_ticks;
int sys_write_console(char *buffer,int size);
int sys_gettime_console();

#endif /* DEVICES_H__*/
