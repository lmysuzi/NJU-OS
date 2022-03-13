#include <common.h>

#define MAGIC 7654321
#define actual(size) (size+sizeof(header_t))

typedef struct node_t{
  size_t size;
  struct node_t *next,*prev;
}node_t;

typedef struct header_t{
  size_t size;
  long magic;
}header_t;

static const size_t maxSize=(16<<20);
static node_t *head;

/*static size_t tableSizeFor(size_t val){
  if (val & (val - 1)){
    val |= val>>1;
    val |= val>>2;
    val |= val>>4;
    val |= val>>8;
    val |= val>>16;
    return val+1;
  }
  else return val == 0 ? 1 : val;
}*/

static void *kalloc(size_t size) {
  if(size<=0||size>maxSize)return NULL;
  //size_t sizePow=tableSizeFor(size);
  node_t *node=head;
  while(node!=NULL){
    if(node->size>=actual(size)){

    }
  }
  return NULL;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  node_t *Head=(node_t *)heap.start;
  printf("%d %d\n",sizeof(header_t),sizeof(node_t));
  Head->next=NULL,Head->size=pmsize-sizeof(node_t);
  head=Head;
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
