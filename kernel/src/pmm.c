#include <common.h>

#define mark printf("fuck\n")

#define PAGENUM  32000
#define PAGESIZE (4096)
#define MINSIZE  (128)
#define MAXSIZE  (16<<20)
#define MAXCPU 8
#define SLABNUM 6

#define orderOfPage(x) (((uint64_t)(x-0x300000))>>12)

enum{
  _128=7,_256,_512,_1024,_2048,_4096,_2p,_4p,_8p,_16p,_32p,_64p,_128p,_256p,_512p,_1024p,_2048p,_4096p
};

static uint8_t sizeOfPage[PAGENUM];

typedef struct lock_t{
  int flag;
}lock_t;

void lockInit(lock_t *lock){
  atomic_xchg(&lock->flag,0);
}

void lock(lock_t *lock){
  while(atomic_xchg(&lock->flag,1)==1);
}

int lock_acquire(lock_t *lock){
  return atomic_xchg(&lock->flag,1);
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

typedef struct header_t{//记录大内存
  void *addr;
  size_t size;
  struct header_t *next,*prev;
}header_t;
header_t *head;
lock_t memoryLock;

typedef struct node_t{//记录slab
  void *addr;
  int blockNum;
  size_t size;
  struct node_t *next;
}node_t;

typedef struct slab_t{
  node_t *head[SLABNUM];
  lock_t  slabLock[SLABNUM];
}slab_t;
static slab_t slab[MAXCPU];

static inline int targetList(size_t size){
  switch (size){
  case 128: return 0;
  case 256: return 1;
  case 512: return 2;
  case 1024: return 3;
  case 2048: return 4;
  case 4096: return 5;
  default:printf("wrong size\n");halt(1);
  }
}

static inline int sizeSpecify(size_t size){
  switch (size){
  case 128: return _128;
  case 256: return _256;
  case 512: return _512;
  case 1024: return _1024;
  case 2048: return _2048;
  case 4096: return _4096;
  case 2*PAGESIZE: return _2p;
  case 4*PAGESIZE: return _4p;
  case 8*PAGESIZE: return _8p;
  case 16*PAGESIZE: return _16p;
  case 32*PAGESIZE: return _32p;
  case 64*PAGESIZE: return _64p;
  case 128*PAGESIZE: return _128p;
  case 256*PAGESIZE: return _256p;
  case 512*PAGESIZE: return _512p;
  case 1024*PAGESIZE: return _1024p;
  case 2048*PAGESIZE: return _2048p;
  case 4096*PAGESIZE: return _4096p;
  default:printf("wrong size\n");halt(1);
  }
}

static void *slab_init(void *pt){
  for(int i=0,n=cpu_count();i<n;i++){
    for(int j=0;j<SLABNUM;j++)lockInit(&slab[i].slabLock[j]);
    for(int j=0,blooksize=128;j<SLABNUM-1;j++,blooksize<<=1){
      slab[i].head[j]=pt;
      slab[i].head[j]->addr=pt;
      slab[i].head[j]->size=10*PAGESIZE;
      slab[i].head[j]->blockNum=10*PAGESIZE/blooksize;
      slab[i].head[j]->next=NULL;
      pt+=10*PAGESIZE;
    }
    slab[i].head[SLABNUM-1]=pt;
    slab[i].head[SLABNUM-1]->addr=pt;
    slab[i].head[SLABNUM-1]->size=1000*PAGESIZE;
    slab[i].head[SLABNUM-1]->blockNum=1000;
    slab[i].head[SLABNUM-1]->next=NULL;
    pt+=1000*PAGESIZE;
  }
  return pt;
}

static void slab_free(void *ptr,size_t size){
  int cpu=cpu_current();
  node_t *node=(node_t *)ptr;
  node->size=size;
  node->addr=ptr;
  node->blockNum=1;
  int target=targetList(size);
  node->next=slab[cpu].head[target];
  slab[cpu].head[target]=node;
}

static void *slab_ask(int cpu,int slabOrder,size_t size){
  node_t *list=slab[cpu].head[slabOrder];
  if(list){
    --list->blockNum;
    void *ans=(void*)list;
    if(!list->blockNum)slab[cpu].head[slabOrder]=slab[cpu].head[slabOrder]->next;
    else{
      node_t *new=(node_t*)(ans+size);
      new->addr=(void *)new;
      new->blockNum=list->blockNum;
      new->size=new->blockNum*size;
      new->next=list->next;
      slab[cpu].head[slabOrder]=new;
    }
    sizeOfPage[orderOfPage(ans)]=sizeSpecify(size);
    return ans;
  }
  return NULL;
}
static void *slab_alloc(size_t size){
  int slabOrder=targetList(size);
  int cpu=cpu_current();
  void *ans=NULL;
  lock(&slab[cpu].slabLock[slabOrder]);
  ans=slab_ask(cpu,slabOrder,size);
  unlock(&slab[cpu].slabLock[slabOrder]);
  if(ans!=NULL)return ans;
  //从其他cpu偷取内存
  cpu=0;
  for(int i=0,n=cpu_count();i<n;i++){
    if(lock_acquire(&slab[cpu].slabLock[slabOrder])==0){
      ans=slab_ask(cpu,slabOrder,size);
      unlock(&slab[cpu].slabLock[slabOrder]);
      if(ans!=NULL)return ans;
    }
  }
  return NULL;
}

static void memory_init(void *ptr){
  head=(header_t*)ptr;
  head->addr=ptr;
  head->size=heap.end-ptr;
  head->next=head->prev=NULL;
  lockInit(&memoryLock);
}

static void *memory_alloc(size_t size){
  int flag=sizeSpecify(size);
  lock(&memoryLock);
  header_t *list=head;
  while(list){
    if(list->size>=size){
      void *iniaddr=(void*)(((size_t)list->addr>>flag)<<flag);   
      void *endaddr=iniaddr+list->size;
      if(iniaddr<list->addr)iniaddr+=size;
      if(iniaddr+size<=endaddr){
        void *ans=iniaddr;
        
        if(list->addr!=iniaddr){//新地址前有剩余
          list->size=iniaddr-list->addr;
          if(iniaddr+size<endaddr){//新地址后有剩余
            header_t *new=(header_t*)(iniaddr+size);
            new->prev=list;
            new->next=list->next;
            new->addr=(void*)new;
            new->size=endaddr-new->addr;
            if(list->next)list->next->prev=new;
            list->next=new;
          }
        }
        else{
          if(iniaddr+size<endaddr){
            header_t *new=(header_t*)(iniaddr+size);
            new->prev=list->prev;
            new->next=list->next;
            new->addr=(void*)new;
            new->size=endaddr-new->addr;
            if(list->prev)list->prev->next=new;
            if(list->next)list->next->prev=new;
            if(list==head)head=new;
          }
          else{
            if(list->prev)list->prev->next=list->next;
            if(list->next)list->next->prev=list->prev;
            if(list==head)head=list->next;
          }
        }
        sizeOfPage[orderOfPage(ans)]=sizeSpecify(size);
        unlock(&memoryLock);
        return ans;
      }
    }
    list=list->next;

  }
  unlock(&memoryLock);
  return NULL;
}

static void memory_merge(){
  header_t *temp=head;
  while(temp){
    begin:
    if(temp->next&&temp->addr+temp->size==temp->next->addr){
      if(temp->next->next)temp->next->next->prev=temp;
      temp->size+=temp->next->size;
      temp->next=temp->next->next;
      goto begin;
    }
    else temp=temp->next;
  }
}

static int freeCount=0;
static void memory_free(void *ptr,size_t size){
  header_t *new=(header_t*)ptr;
  new->size=size;
  new->addr=ptr;
  lock(&memoryLock);
  if(new<head){
    new->next=head,new->prev=NULL;
    head->prev=new,head=new;
    goto end;
  }
  header_t *temp=head;
  while(temp){
    if(temp->next==NULL||temp->next>new){
      new->prev=temp,new->next=temp->next;
      if(temp->next)temp->next->prev=new;
      temp->next=new;
      goto end;
    }
    temp=temp->next;
  }
  end:
  if(++freeCount==1)freeCount=0,memory_merge();
  unlock(&memoryLock);
}

static void *kalloc(size_t size) {
  size=tableSizeFor(size);
  if(size<MINSIZE)size=MINSIZE;
  if(size>MAXSIZE)return NULL;
  else if(size>=MINSIZE&&size<=PAGESIZE)return slab_alloc(size);
  else if(size<=MAXSIZE&&size>PAGESIZE)return memory_alloc(size);
  return NULL;
}

static void kfree(void *ptr){
  size_t size=1<<sizeOfPage[orderOfPage(ptr)];
  if(size<=PAGESIZE&&size>=MINSIZE)slab_free(ptr,size);
  else if(size>PAGESIZE)memory_free(ptr,size);
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  void *pt=heap.start;
  pt=slab_init(pt);
  memory_init(pt);
  /*void *fuck=kalloc(16<<16);
  printf("%x\n",fuck);
  kfree(fuck);
  header_t *yin=head;
  while(yin){
    printf("%x %x %x\n",yin->addr,yin->size,yin->addr+yin->size);
    yin=yin->next;
  }*/
  /*node_t *temp=slab[cpu_current()].head[0];
  while(temp){
    printf("%x %d\n",temp->addr,temp->blockNum);
    temp=temp->next;
  }*/
  /*printf("%x\n",kalloc(9));
  printf("%x\n",kalloc(9));
  printf("%x\n",kalloc(1025));
  printf("%x\n",kalloc(1025));
  printf("%x\n",kalloc(3098));*/
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};