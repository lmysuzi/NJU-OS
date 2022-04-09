#include <common.h>
#include <os.h>

irq_t *irq_head;

static void os_init() {
  irq_head=NULL;
  dev->init();
  pmm->init();
  kmt->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  while (1) ;
}

static Context *os_trap(Event ev, Context *context){
  return NULL;
}

static void os_on_irq(int seq, int event, handler_t handler){

}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
