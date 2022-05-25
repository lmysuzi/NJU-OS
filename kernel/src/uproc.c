#include <os.h>
#include <syscall.h>

#include "initcode.inc"

static void init(){

}

static int kputc(task_t *task, char ch){
  return 0;
}

static int fork(task_t *task){
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
static int getpid(task_t *task){
  return 0;

}

static int sleep(task_t *task, int seconds){
  return 0;

}

static int64_t uptime(task_t *task){
  return 0;

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
