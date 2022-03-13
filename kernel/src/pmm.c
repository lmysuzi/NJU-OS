#include <common.h>

#define MAGIC 7654321

typedef struct node_t{
  uintptr_t size;
  struct node_t *next;
}node_t;

typedef struct header_t{
  uintptr_t size;
  int magic;
}header_t;

static node_t **head;

static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  node_t *Head=(node_t *)heap.start;
  Head->next=NULL,Head->size=pmsize-sizeof(node_t);
  head=&Head;
  printf("%p\n",*head);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
