#include <os.h>
#include <syscall.h>

#include <kmt.h>
#include <uproc.h>

#include "initcode.inc"

#define debug printf("shit\n")

typedef void *(*pgalloc_type)(int);


spinlock_t pglock;

static void
pgmap(task_t *task,void *va, void *pa){
  task->pa[task->np]=pa;
  task->va[task->np]=va;
  task->np++;

  map(&task->as,va,pa,MMAP_READ|MMAP_WRITE);

}


void
pgfault(Event ev,Context *context){
  kmt->spin_lock(&pglock);

  AddrSpace *as=&(task_now()->as);
  void *pa=pmm->alloc(as->pgsize);
  void *va=(void*)(ev.ref&~(as->pgsize-1L));

  if(va==as->area.start)memcpy(pa,_init,_init_len);

  pgmap(task_now(),va,pa);

  kmt->spin_unlock(&pglock);
}

static void 
init(){
  vme_init((pgalloc_type)pmm->alloc,pmm->free);

  kmt->init(&pglock);

  //ucreate(pmm->alloc(sizeof(task_t)),"u");

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


static int 
wait(task_t *task, int *status){
  return 0;
}


static int
exit(task_t *task, int status){
  return 0;
}


static int 
kill(task_t *task, int pid){
  return 0;

}


static void *
mmap(task_t *task, void *addr, int length, int prot, int flags){
  return NULL;
}


static int 
getpid(task_t *task){
  return task_now()->id;
}


static int 
sleep(task_t *task, int seconds){
  int us=io_read(AM_TIMER_UPTIME).us+seconds*1000000;
  sleep_insert(task_now(),us);
  yield();
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