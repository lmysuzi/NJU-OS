#include <common.h>
#include <kmt.h>

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)
#define MAX_CPU 8

static task_t *task_head=NULL;
static spinlock_t task_lock;

static task_t *currents[MAX_CPU];
static task_t *idles[MAX_CPU];
#define current currents[cpu_current()]
#define idle idles[cpu_current()]
enum{
  TASK_READY=1,TASK_RUNNING,TASK_SLEEP,
};

static void inline task_insert(task_t *task){
  panic_on(task_lock.flag==0,"wrong lock");

  task->prev=NULL,task->next=task_head;
  if(task_head!=NULL)task_head->prev=task;
  task_head=task;
}


static void inline task_delete(task_t *task){
  panic_on(task_lock.flag==0,"wrong lock");

  if(task->next)task->next->prev=task->prev;
  if(task->prev)task->prev->next=task->next;
  else task_head=task_head->next;
  pmm->free_safe(task->kstack);
  pmm->free_safe(task);
}


static void spin_lock(spinlock_t *lk);
static void spin_unlock(spinlock_t *lk);
static int create(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
static void teardown(task_t *task);


static Context *kmt_context_save(Event ev,Context *context){
  panic_on(current==NULL,"current is null");
  current->context=context;
  return NULL;
}


static Context *kmt_schedule(Event ev,Context *context){
  panic_on(current==NULL,"current is null");

  spin_lock(&task_lock);
  if(task_head==NULL){
    panic_on(current!=idle,"wrong current");
    spin_unlock(&task_lock);
    return current->context;
  }
  task_t *task=current->next;//if current == idle , then task is NULL too
  if(task==NULL)task=task_head;
  task_t *task_begin=task;

  current->status=TASK_READY;

  do{
    if(task->status==TASK_READY)break;
    if(task->next)task=task->next;
    else task=task_head;
  }while(task!=task_begin);

  current=task;
  current->status=TASK_RUNNING;

  spin_unlock(&task_lock);
  return context;
}


static void spin_init(spinlock_t *lk, const char *name){
  lk->flag=0;lk->name=name;
}

static void spin_lock(spinlock_t *lk){
  bool prev_status=ienabled();
  iset(false);
  while(atomic_xchg(&lk->flag,1)==1);
  lk->status=prev_status;
}


static void spin_unlock(spinlock_t *lk){
  panic_on(ienabled()==1,"wrong status");
  atomic_xchg(&lk->flag,0);
  iset(lk->status);
}



static void init(){
  spin_init(&task_lock,"task_lock");

  for(int cpu=0;cpu<cpu_count();cpu++){
    idles[cpu]=pmm->alloc(sizeof(task_t));

    panic_on(idles[cpu]==NULL,"alloc fail");

    idles[cpu]->next=idles[cpu]->prev=NULL;
    currents[cpu]=idles[cpu];
  }

  os->on_irq(INT_MIN,EVENT_NULL,kmt_context_save);
  os->on_irq(INT_MAX,EVENT_NULL,kmt_schedule);
}
    

static int create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  panic_on(task==NULL,"task is NULL");

  task->name=name;
  task->status=TASK_READY;
  task->kstack=pmm->alloc_safe(KSTACK_SIZE);
  panic_on(task->kstack==NULL,"not enough space for kstack");

  Area kstack={
    .start=(void *)task->kstack,
    .end=((void *)task->kstack)+KSTACK_SIZE,
  };
  task->context=kcontext(kstack,entry,arg);

  spin_lock(&task_lock);
  task_insert(task);
  spin_unlock(&task_lock);

  return 0;
}


static void teardown(task_t *task){
  panic_on(task==NULL,"task is NULL");

  spin_lock(&task_lock);
  task_delete(task);
  spin_unlock(&task_lock);
}


static void sem_init(sem_t *sem, const char *name, int value){

}


static void sem_wait(sem_t *sem){

}


static void sem_signal(sem_t *sem){

}

MODULE_DEF(kmt)={
  .init=init,
  .create=create,
  .teardown=teardown,
  .spin_init=spin_init,
  .spin_lock=spin_lock,
  .spin_unlock=spin_unlock,
  .sem_init=sem_init,
  .sem_wait=sem_wait,
  .sem_signal=sem_signal,
};