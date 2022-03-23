#include <common.h>

#define PAGESIZE 4096


typedef struct lock_t{
  int flag;
}lock_t;

void lockInit(lock_t *lock){
  atomic_xchg(&lock->flag,0);
}

void lock(lock_t *lock){
  while(atomic_xchg(&lock->flag,1)==1);
}

void unlock(lock_t *lock){
  atomic_xchg(&lock->flag,0);
}



static size_t tableSizeFor(size_t val){
  if (val & (val - 1)){
    val |= val>>1;
    val |= val>>2;
    val |= val>>4;
    val |= val>>8;
    val |= val>>16;
    return val+1;
  }
  else return val == 0 ? 1 : val;
}


static void *kalloc(size_t size) {
  size=tableSizeFor(size);
  return NULL;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  printf("%d\n",pmsize/PAGESIZE);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};