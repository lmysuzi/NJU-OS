#include <common.h>

#define MAGIC 7654321
#define COUNT 10
#define actual(size) (size+sizeof(node_t))
#define headerAddr(addr) ((void*)addr-sizeof(node_t))

#define mark printf("fuck\n")

typedef struct lock_t{
  int flag;
}lock_t;

int testAndSet(int *oldPtr,int new){
  int old=*oldPtr;
  *oldPtr=new;
  return old;
}

void init(lock_t *lock){
  lock->flag=0;
}

void lock(lock_t *lock){
  while(testAndSet(&lock->flag,1)==1);
}

void unlock(lock_t *lock){
  lock->flag=0;
}

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
static lock_t pmmLock;
static int count=0;//记录free的次数，定期merge

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

/*static int mergeL(node_t *new){
  node_t *temp=head;
  while(temp){
    if((void*)temp+temp->size+sizeof(node_t)==(void*)new){
      temp->size+=sizeof(node_t)+new->size;
      return 0;
    }
    temp=temp->next;
  }
  return -1;
}*/

/*static int mergeR(node_t *new){
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
}*/

static void insert(node_t *new){
  if(new<head){
    new->next=head,new->prev=NULL;
    head->prev=new;head=new;
    return;
  }
  node_t *temp=head;
  while(temp!=NULL){
    if(temp->next==NULL||temp->next>new){
      new->next=temp->next,new->prev=temp;
      if(temp->next!=NULL)temp->next->prev=new;
      temp->next=new;
      return;
    }
    temp=temp->next;
  }
}

static void merge(){
  node_t *temp=head;
  while(temp){
    printf("%p %x %x\n",temp,temp->size,(void*)temp+temp->size+sizeof(node_t));
    temp=temp->next;
  }
    mark;
  temp=head;
  while(temp){
    if(temp->next&&(void*)temp+actual(temp->size)==(void*)temp->next){
      temp->size+=actual(temp->next->size);
      temp->next=temp->next->next;
    }
    else temp=temp->next;
  }
  temp=head;
  while(temp){
    printf("%p %x %x\n",temp,temp->size,(void*)temp+temp->size+sizeof(node_t));
    temp=temp->next;
  }
  printf("\n");
}

static void *kalloc(size_t size) {
  if(size<=0||size>maxSize)return NULL;
  size_t sizePow=tableSizeFor(size);
  size_t mask=~((size_t)sizePow-1);
  node_t *node=head;
  lock(&pmmLock);
  while(node!=NULL){
    if(node->size>=actual(size)){
      void *iniAddr=(void*)node+sizeof(node_t);
      void *endAddr=iniAddr+node->size;
      void *addr=(void*)(mask&(size_t)iniAddr);
      for(;addr<endAddr;addr+=size){
        //若申请到的内存块为head，可能会存在大量内存浪费
        if(addr>=iniAddr){
          if(addr+actual(size)<endAddr){//加上node_t以提供新的空闲内存的节点
            node_t *newAddr=(node_t*)(addr+size);
            newAddr->size=endAddr-(void*)newAddr-sizeof(node_t);
            newAddr->next=node->next,newAddr->prev=node->prev;
            if(node==head)head=newAddr;
            if(node->prev){
              node->prev->next=newAddr;
              node->prev->size=addr-(void*)node->prev-2*sizeof(node_t);
            }
            if(node->next)node->next->prev=newAddr;
          }
          else{//该内存块分完
            if(head==node)head=node->next;
            if(node->prev)node->prev->next=node->next,node->prev->size=addr-(void*)node->prev-2*sizeof(node_t);
            if(node->next)node->next->prev=node->prev;
          }
          header_t *header=(header_t*)(addr-sizeof(node_t));
          header->size=size,header->magic=MAGIC;
          unlock(&pmmLock);
          return addr;
        }
      }
    }
    node=node->next;
  }
  unlock(&pmmLock);
  return NULL;
}

static void kfree(void *ptr) {
  lock(&pmmLock);
  header_t *header=headerAddr(ptr);
  if(header->magic!=MAGIC)printf("wrong!\n"),halt(1);
  size_t size=header->size;
  node_t *new=(node_t*)header;
  new->size=size;
  insert(new);
  if((++count)==COUNT)count=0,merge();
  unlock(&pmmLock);
}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  node_t *Head=(node_t *)heap.start;
  init(&pmmLock);
  //printf("%d %d\n",sizeof(header_t),sizeof(node_t));
  Head->prev=NULL,Head->next=NULL,Head->size=pmsize-sizeof(node_t);
  head=Head;
  /*int* a=kalloc(9);
  int* b=kalloc(32);
  printf("%p %p\n\n",a,head);
  kfree(b);
  kfree(a);
  node_t *temp=head;
  while(temp){
    printf("%p %x %x\n",temp,temp->size,(void*)temp+temp->size+sizeof(node_t));
    temp=temp->next;
  }
  merge();
  temp=head;
  mark;
  while(temp){
    printf("%p %x %x\n",temp,temp->size,(void*)temp+temp->size+sizeof(node_t));
    temp=temp->next;
  }*/
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
