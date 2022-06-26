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

  printf("map: %p -> %p\n",va,pa);
  map(&task->as,va,pa,MMAP_READ|MMAP_WRITE);
}


static Context*
pgfault(Event ev,Context *context){
  kmt->spin_lock(&pglock);

  AddrSpace *as=&(task_now()->as);
  void *pa=pmm->alloc(as->pgsize);
  void *va=(void*)(ev.ref&~(as->pgsize-1L));

  if(va==as->area.start)memcpy(pa,_init,_init_len);

  pgmap(task_now(),va,pa);

  kmt->spin_unlock(&pglock);

  return NULL;
}


int
syscall(Context *context){
  int ret=0;
  //iset(true);

  //printf("%d\n",context->GPRx);

  switch(context->GPRx){
    case SYS_kputc:{
      ret=uproc->kputc(NULL,(char)context->GPR1);
    }break;

    case SYS_fork:{
      ret=uproc->fork(NULL);
    }break;

    case SYS_exit :{
      ret=uproc->exit(NULL,context->GPR1);
    }break;

    case SYS_wait:{
      ret=uproc->wait(NULL,(int*)context->GPR1);
    }break;

    case SYS_getpid:{
      ret=uproc->getpid(NULL);
    }break;

    case SYS_sleep:{
      ret=uproc->sleep(NULL,context->GPR1);
    }break;

    case SYS_uptime:{
      ret=uproc->uptime(NULL);
    }break;
      
    //default :printf("%d\n",context->GPRx);panic("wrong event");
  }

  //iset(false);
  return ret;
}


static Context *
uproc_syscall(Event ev,Context *context){
  task_now()->context->GPRx=syscall(context);
  return NULL;
}


static Context *
uproc_error(Event ev,Context *context){
  assert(0);
  return NULL;
}

static void 
init(){
  vme_init((pgalloc_type)pmm->alloc,pmm->free);

  kmt->spin_init(&pglock,"pglock");

  os->on_irq(INT_MIN+1,EVENT_ERROR,uproc_error);
  os->on_irq(INT_MIN+2,EVENT_PAGEFAULT,pgfault);
  os->on_irq(INT_MIN+3,EVENT_SYSCALL,uproc_syscall);

  ucreate(pmm->alloc(sizeof(task_t)),"u",0);

}


static int 
kputc(task_t *task, char ch){
  putch(ch);
  return 0;
}


static int 
fork(task_t *task){
  //iset(false);
  task_t *child_task=pmm->alloc(sizeof(task_t));
  ucreate(child_task,NULL,task_now()->id);

  uintptr_t rsp0=child_task->context->rsp0;
  void *cr3=child_task->context->cr3;


  for(int i=0;i<task_now()->np;i++){
    void *va=task_now()->va[i];
    void *pa=task_now()->pa[i];
    void *npa=pmm->alloc(task_now()->as.pgsize);
    memcpy(npa,pa,task_now()->as.pgsize);
    pgmap(child_task,va,npa);
  }

  memcpy((void*)child_task->context,(void*)task_now()->context,sizeof(Context));
  //child_task->context=task_now()->context;
  child_task->context->rsp0=rsp0;
  child_task->context->cr3=cr3;
  child_task->context->GPRx=0;
  child_task->np=task_now()->np;

 // panic_on(child_task->status!=TASK_RUNNING,"wrong child status");
  child_task->status=TASK_READY;

  //iset(true);
  return child_task->id;
}


static int 
wait(task_t *task, int *status){
  return 0;
}


static int
exit(task_t *task, int status){
  task_now()->status=TASK_DEAD;
  return status;
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
  //yield();
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