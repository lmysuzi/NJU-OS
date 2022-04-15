#include <common.h>
#include <kmt.h>

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)

task_t *task_head=NULL;
spinlock_t task_lock;

static Context *kmt_context_save(Event ev,Context *context){
  return NULL;
}

static Context *kmt_schedule(Event ev,Context *context){
  return NULL;
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
  os->on_irq(INT_MIN,EVENT_NULL,kmt_context_save);
  os->on_irq(INT_MAX,EVENT_NULL,kmt_schedule);
}
    

static int create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  panic_on(task==NULL,"task is NULL");

  task->name=name;
  task->kstack=pmm->alloc_safe(KSTACK_SIZE);

  panic_on(task->kstack==NULL,"not enough space for kstack");

  Area kstack={
    .start=(void *)task->kstack,
    .end=((void *)task->kstack)+KSTACK_SIZE,
  };
  task->context=kcontext(kstack,entry,arg);

  spin_lock(&task_lock);
  task->prev=NULL,task->next=task_head;
  if(task_head!=NULL)task_head->prev=task;
  task_head=task;
  spin_unlock(&task_lock);

  return 0;
}


static void teardown(task_t *task){
  panic_on(task==NULL,"task is NULL");

  pmm->free_safe(task->kstack);

  spin_lock(&task_lock);
  if(task->next)task->next->prev=task->prev;
  if(task->prev)task->prev->next=task->next;
  if(task==task_head)task=task->next;
  spin_unlock(&task_lock);

}



MODULE_DEF(kmt)={
  .init=init,
  .create=create,
  .teardown=teardown,
  .spin_init=spin_init,
  .spin_lock=spin_lock,
  .spin_unlock=spin_unlock,
};