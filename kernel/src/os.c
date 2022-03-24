#include <common.h>

static void os_init() {
  pmm->init();
}

static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  void*a[19];
  for(int i=0;i<19;i++)a[i]=NULL;
  for(int i=50;i;i--){
    a[i]=pmm->alloc(128);
    printf("%x\n",a[i]);
  }
  for(int i=50;i;i--){
    if(a[i])
    pmm->free(a[i]);
  }
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
