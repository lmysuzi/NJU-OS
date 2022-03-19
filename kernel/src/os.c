#include <common.h>

static void os_init() {
  pmm->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void*a[19];
  for(int i=1;i<=15;i++){
    a[i]=pmm->alloc(128*i);
    memset(a[i],0x1111,i*128);
  }
  for(int i=15;i;i--){
    pmm->free(a[i]);
  }
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
