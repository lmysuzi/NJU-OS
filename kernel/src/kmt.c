#include <common.h>
#include <kmt.h>
#include <os.h>

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)
#define MAX_CPU 8

#define mark printf("fuck\n")


static int cpu_sched; //下一个线程调动的cpu
static spinlock_t lock_cpu_sched;
static spinlock_t task_locks[MAX_CPU];
static task_t *tasks[MAX_CPU];
static task_t *currents[MAX_CPU];
static task_t *idles[MAX_CPU];
static int task_nums[MAX_CPU];

#define current currents[cpu_current()]
#define idle idles[cpu_current()]
#define task_lock task_locks[cpu_current()]
#define head tasks[cpu_current()]
#define task_num task_nums[cpu_current()]
#define head_for(_) tasks[_->which_cpu]
#define lock_for(_) task_locks[_->which_cpu]
#define task_num_for(_) task_nums[_->which_cpu]

enum{
  TASK_READY=1,TASK_RUNNING,TASK_SLEEP,
};



static void inline 
task_insert(task_t *task){
  panic_on(lock_for(task).flag==0,"wrong lock");


  task->prev=NULL,task->next=head_for(task);
  if(head_for(task)!=NULL)head_for(task)->prev=task;
  head_for(task)=task;
  task_num_for(task)++;

}


static void inline 
task_delete(task_t *task){
  panic_on(lock_for(task).flag==0,"wrong lock");
  
  task_num_for(task)--;

  if(task->next)task->next->prev=task->prev;
  if(task->prev)task->prev->next=task->next;
  else head_for(task)=task->next;
  pmm->free(task->kstack);
  pmm->free(task);
}


static void spin_lock(spinlock_t *lk);
static void spin_unlock(spinlock_t *lk);
static int create(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
static void teardown(task_t *task);


static Context *
kmt_context_save(Event ev,Context *context){
  if(!current)current=idle;
  else current->context=context;

  return NULL;
}


static Context *
kmt_schedule(Event ev,Context *context){
  panic_on(current==NULL,"current is null");

  spin_lock(&task_lock);

  if(head==NULL){
    panic_on(current!=idle,"wrong current");
    spin_unlock(&task_lock);
    return current->context;
  }

  task_t *task=current->next;//if current == idle , then task is NULL too

  if(task==NULL)task=head;

  if(current->status==TASK_RUNNING){
    current->status=TASK_READY;
  }


  int round=rand()%task_num;
  for(int i=0;i<round;i++){
    if(task->next)task=task->next;
    else task=head;
  }

  /*while(task->status!=TASK_READY){
    if(task->next)task=task->next;
    else task=head;
  }*/

  //if(task->status==TASK_READY)current=task;
  //else current=idle;
  task_t *task_begin=task;
  do{
    if(task->status==TASK_READY)break;
    if(task->next)task=task->next;
    else task=head;
  }while(task!=task_begin);

  current=task;
  if(current->status!=TASK_READY)current=idle;
  current->status=TASK_RUNNING;

  spin_unlock(&task_lock);
  return current->context;
}


static void 
spin_init(spinlock_t *lk, const char *name){
  lk->flag=0;lk->name=name;
}

static void 
spin_lock(spinlock_t *lk){
  bool prev_status=ienabled();
  iset(false);
  while(atomic_xchg(&lk->flag,1)==1);//printf("%s\n",lk->name);
  lk->status=prev_status;
}


static void 
spin_unlock(spinlock_t *lk){
  panic_on(ienabled()==1,"wrong status");
  atomic_xchg(&lk->flag,0);
  iset(lk->status);
}


static void 
idle_task(){
  while(1);//printf("fuck\n");
  panic("should not reach");
}




static void 
init(){
  spin_init(&lock_cpu_sched,"lock_cpu_sched");
  cpu_sched=0;

  for(int cpu=0;cpu<cpu_count();cpu++){
    spin_init(&task_locks[cpu],"task_lock");
    tasks[cpu]=NULL;
    currents[cpu]=NULL;
    task_nums[cpu]=0;

    idles[cpu]=pmm->alloc(sizeof(task_t));

    panic_on(idles[cpu]==NULL,"alloc fail");

    idles[cpu]->kstack=pmm->alloc(KSTACK_SIZE);
    panic_on(idles[cpu]->kstack==NULL,"not enough space for kstack");

    Area kstack={
      .start=(void *)idles[cpu]->kstack,
      .end=((void *)idles[cpu]->kstack)+KSTACK_SIZE,
    };

    idles[cpu]->context=kcontext(kstack,idle_task,NULL);
    idles[cpu]->next=idles[cpu]->prev=NULL;
    idles[cpu]->status=TASK_RUNNING;
  }

  os->on_irq(INT_MIN,EVENT_NULL,kmt_context_save);
  os->on_irq(INT_MAX,EVENT_NULL,kmt_schedule);
}
    



static int 
create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  panic_on(task==NULL,"task is NULL");

  task->name=name;
  task->status=TASK_READY;
  task->kstack=pmm->alloc(KSTACK_SIZE);
  panic_on(task->kstack==NULL,"not enough space for kstack");

  Area kstack={
    .start=(void *)task->kstack,
    .end=((void *)task->kstack)+KSTACK_SIZE,
  };
  task->context=kcontext(kstack,entry,arg);

  spin_lock(&lock_cpu_sched);
  task->which_cpu=cpu_sched;
  cpu_sched=(cpu_sched+1)%cpu_count();
  spin_unlock(&lock_cpu_sched);

  spin_lock(&lock_for(task));
  task_insert(task);
  spin_unlock(&lock_for(task));

  //printf("Task %s has been created on the cpu %d\n",name,task->which_cpu);

  return 0;
}




static void 
teardown(task_t *task){
  panic_on(task==NULL,"task is NULL");

  spin_lock(&task_locks[task->which_cpu]);
  task_delete(task);
  spin_unlock(&task_locks[task->which_cpu]);
}


static void inline 
sem_task_insert(sem_t *sem, task_t *task){
  panic_on(sem==NULL,"sem is null");
  panic_on(task==NULL,"task is null");

  sem_tasks_t *sem_task_node=pmm->alloc(sizeof(sem_tasks_t));
  panic_on(sem_task_node==NULL,"alloc fail");

  sem_task_node->task=task;
  sem_task_node->prev=NULL;
  sem_task_node->next=sem->sem_tasks;
  if(sem->sem_tasks!=NULL)sem->sem_tasks->prev=sem_task_node;
  sem->sem_tasks=sem_task_node;

  spin_lock(&lock_for(task));
  task->status=TASK_SLEEP;
  spin_unlock(&lock_for(task));
}



static void inline 
sem_task_delete(sem_t *sem){
  panic_on(sem==NULL,"sem is null");
  panic_on(sem->sem_tasks==NULL,"sem_tasks is null");

  sem_tasks_t *sem_task_node=sem->sem_tasks;
  while(sem_task_node->next!=NULL)sem_task_node=sem_task_node->next;
  panic_on(sem_task_node->next!=NULL,"wrong task");
  
  if(sem_task_node->prev!=NULL)sem_task_node->prev->next=NULL;
  else sem->sem_tasks=NULL;
  
  spin_lock(&lock_for(sem_task_node->task));
  sem_task_node->task->status=TASK_READY;
  spin_unlock(&lock_for(sem_task_node->task));

  pmm->free(sem_task_node);
}



static void 
sem_init(sem_t *sem, const char *name, int value){
  panic_on(sem==NULL,"sem is null");
  
  sem->name=name;
  sem->count=value;
  sem->sem_tasks=NULL;
  spin_init(&sem->lock,name);
}



static void 
sem_wait(sem_t *sem){
  panic_on(sem==NULL,"sem is null");
  spin_lock(&sem->lock);
  sem->count--;

  if(sem->count<0){
    sem_task_insert(sem,current);
    spin_unlock(&sem->lock);
    yield();
  }
  else spin_unlock(&sem->lock);
}



static void 
sem_signal(sem_t *sem){
  panic_on(sem==NULL,"sem is null");

  spin_lock(&sem->lock);
  sem->count++;
  
  if(sem->sem_tasks!=NULL){
    sem_task_delete(sem);
  }

  spin_unlock(&sem->lock);
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