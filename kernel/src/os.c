#include <common.h>
#include <os.h>
#include <kmt.h>
#include <uproc.h>

static irq_t *irq_head;
static spinlock_t irq_lock;



/*sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal

void producer(void *arg) { while (1) { P(&empty); printf(" %d",(size_t)arg);putch('('); V(&fill);  } }
void consumer(void *arg) { while (1) { P(&fill);  printf(" %d",(size_t)arg);putch(')'); V(&empty); } }

void f(){
  for (int i = 0; i < 4; i++) // 4 个生产者
    kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, (void*)(size_t)i);
  while(1)yield();
}

void g(){
  for (int i = 0; i < 5; i++) // 5 个消费者
    kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, (void*)(size_t)i);
  while(1)yield();
}*/

/*void time(){
  while(1){
    //printf("%d\n",uproc->uptime(NULL));
    printf("%d\n",uproc->getpid(NULL));
    uproc->sleep(NULL,1);
    yield();
  }
}*/

static void os_init() {
  irq_head=NULL;kmt->spin_init(&irq_lock,"irq_lock");
  pmm->init();
  kmt->init();
  uproc->init();
  //dev->init();
  /*kmt->sem_init(&empty, "empty", 5);  // 缓冲区大小为 5
  kmt->sem_init(&fill,  "fill",  0);
  kmt->create(pmm->alloc(sizeof(task_t)),"fuck",f,NULL);
  kmt->create(pmm->alloc(sizeof(task_t)),"fuck",g,NULL);*/
 // kmt->create(pmm->alloc(sizeof(task_t)),"time",time,NULL);
}



static void os_run() {
  /*for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }*/

  iset(true);
  while (1)printf("a");
}



static Context *os_trap(Event ev, Context *context){
  panic_on(ienabled(),"wrong status");
  //panic_on(context==NULL,"context is null");

  //printf("%d\n",ev.event);

 // printf("fuck%d\n",ev.event);
  //yield();
  /*if(!test){
    test++;
    con=context;
    printf("en");
  //  yield();
  }
 // if(con==context)printf("yes\n");*/
  
  //iset(false);
 /* while(1){
    printf("fuci");
    yield();
  }*/

 /* irq_t *temp=irq_head;
  while(temp){
    printf("%d ",temp->seq);
    temp=temp->next;
  }printf("\n");*/

  

  Context *next=NULL;
  //kmt->spin_lock(&irq_lock);
  irq_t *irq=irq_head;
  while(irq!=NULL){
    if(irq->event==EVENT_NULL||irq->event==ev.event){
      Context *r=irq->handler(ev,context);
      panic_on(r&&next, "returning multiple contexts");
      if(r)next=r;
    }
    irq=irq->next;
  }
  //kmt->spin_unlock(&irq_lock);
  panic_on(!next, "returning NULL context");
  return next;
}



static void os_on_irq(int seq, int event, handler_t handler){
  irq_t *new_irq=pmm->alloc(sizeof(irq_t));
  panic_on(new_irq==NULL,"irq alloc fail");

  new_irq->seq=seq,new_irq->event=event,new_irq->handler=handler;

  kmt->spin_lock(&irq_lock);

  if(irq_head==NULL){
    new_irq->next=NULL;irq_head=new_irq;
    kmt->spin_unlock(&irq_lock);return;
  }

  if(irq_head->seq>seq){
    new_irq->next=irq_head;irq_head=new_irq;
    kmt->spin_unlock(&irq_lock);return;
  }

  irq_t *temp=irq_head;
  while(temp!=NULL){
    if(temp->next==NULL||temp->next->seq>seq){
      new_irq->next=temp->next;temp->next=new_irq;
      kmt->spin_unlock(&irq_lock);return;
    }
    temp=temp->next;
  }
  panic("should not reach here");
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
