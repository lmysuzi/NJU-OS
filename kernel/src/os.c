#include <common.h>
#include <os.h>

static irq_t *irq_head;
static spinlock_t irq_lock;

static void os_init() {
  irq_head=NULL;kmt->spin_init(&irq_lock,NULL);
  pmm->init();
  kmt->init();
}



static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }

  iset(true);
  while (1)printf("fuck\n");
}



static Context *os_trap(Event ev, Context *context){
  panic_on(ienabled(),"wrong status");
  panic_on(context==NULL,"context is null");

  Context *next=NULL;

  kmt->spin_lock(&irq_lock);
  irq_t *irq=irq_head;
  while(irq!=NULL){
    if(irq->event==EVENT_NULL||irq->event==ev.event){
      Context *r=irq->handler(ev,context);
      panic_on(r&&next, "returning multiple contexts");
      if(r)next=r;
    }
  }
  kmt->spin_unlock(&irq_lock);

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

  if(irq_head->seq<seq){
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
