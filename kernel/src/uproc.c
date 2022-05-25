#include <os.h>
#include <syscall.h>

#include "initcode.inc"

typedef void *(*pgalloc_type)(int);

AM_TIMER_UPTIME_T time;

static void 
init(){
  vme_init((pgalloc_type)pmm->alloc,pmm->free);
}


static int 
kputc(task_t *task, char ch){
  putch(ch);
  return 0;
}


static int 
fork(task_t *task){
  return 0;
}


static int wait(task_t *task, int *status){
  return 0;

}

static int exit(task_t *task, int status){
  return 0;
}
static int kill(task_t *task, int pid){
  return 0;

}
static void *mmap(task_t *task, void *addr, int length, int prot, int flags){
  return NULL;
}


static int 
getpid(task_t *task){
  return task->id;
}

static int sleep(task_t *task, int seconds){
  return 0;
}


static int64_t 
uptime(task_t *task){
  int us=io_read(AM_TIMER_UPTIME).us;
  return us/1000;
}


MODULE_DEF(uproc) = {
  .init=init,
  .kputc=kputc,
  .fork=fork,
  .wait=wait,
  .exit=exit,
  .kill=kill,
  .mmap=mmap,
  .getpid=getpid,
  .sleep=sleep,
  .uptime=uptime,
};
