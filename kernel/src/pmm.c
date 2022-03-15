#include <common.h>

#define MAGIC 7654321
#define actual(size) (size+sizeof(node_t))
#define headerAddr(addr) ((void*)addr-sizeof(node_t))

#define mark printf("fuck")

typedef struct node_t{
  size_t size;
  struct node_t *next,*prev;
}node_t;

typedef struct header_t{
  size_t size;
  int magic;
}header_t;

static const size_t maxSize=(16<<20);
static node_t *head;

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

static int mergeL(node_t *new){
  node_t *temp=head;
  while(temp){
    if((void*)temp+temp->size+sizeof(node_t)==(void*)new){
      temp->size+=sizeof(node_t)+new->size;
      return 0;
    }
    temp=temp->next;
  }
  return -1;
}

static int mergeR(node_t *new){
  node_t *temp=head;
  while(temp){
    if((void*)new+new->size+sizeof(node_t)==(void*)temp){
      new->size+=temp->size+sizeof(node_t);
      new->next=temp->next;
      new->prev=temp->prev;
      if(temp==head)head=new;
      if(temp->prev)temp->prev->next=new;
      if(temp->next)temp->next->prev=new;
      return 0;
    }
    temp=temp->next;
  }
  return -1;
}

static void *kalloc(size_t size) {
  if(size<=0||size>maxSize)return NULL;
  size_t sizePow=tableSizeFor(size);
  size_t mask=~((size_t)sizePow-1);
  node_t *node=head;
  while(node!=NULL){
    if(node->size>=actual(size)){
      void *iniAddr=(void*)node+sizeof(node_t);
      void *endAddr=iniAddr+node->size;
      void *addr=(void*)(mask&(size_t)iniAddr);
      for(;addr<iniAddr+node->size;addr+=sizePow){
        if(addr>=iniAddr){
          if(addr+size+sizeof(node_t)<endAddr){
            node_t *newAddr=(node_t*)(addr+size);
            newAddr->size=endAddr-(void*)newAddr-sizeof(node_t);
            newAddr->next=node->next,newAddr->prev=node->prev;
            if(node==head)head=newAddr;
            if(node->prev)node->prev->next=newAddr;
            if(node->next)node->next->prev=newAddr;
          }
          header_t *header=(header_t*)(addr-sizeof(node_t));
          header->size=size,header->magic=MAGIC;
          return addr;
        }
      }
    }
    node=node->next;
  }
  return NULL;
}

static void kfree(void *ptr) {
  header_t *header=headerAddr(ptr);
  if(header->magic!=MAGIC)printf("wrong!\n"),halt(1);
  size_t size=header->size;
  node_t *new=(node_t*)header;
  new->size=size;
  if(mergeR(new)==-1)
    if(mergeL(new)==-1){
      new->next=head->next;
      new->prev=head->prev;
      head->prev=new;
      head=new;
      mark;
    }
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  node_t *Head=(node_t *)heap.start;
  printf("%d %d\n",sizeof(header_t),sizeof(node_t));
  Head->prev=NULL,Head->next=NULL,Head->size=pmsize-sizeof(node_t);
  head=Head;
  int* a=kalloc(sizeof(int));
  printf("%p %p\n",a,head);
  kfree(a);
  printf("%p %p\n",a,head);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
