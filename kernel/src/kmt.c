#include <common.h>
#include <kmt.h>

static void init(){

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

  return 0;
}

static void teardown(task_t *task){

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

MODULE_DEF(kmt)={
  .init=init,
  .create=create,
  .teardown=teardown,
  .spin_init=spin_init,
  .spin_lock=spin_lock,
  .spin_unlock=spin_unlock,
};