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

typedef struct node_t{//记录slab
  void *addr;
  int blockNum;
  size_t size;
  struct node_t *next;
}node_t;

typedef struct slab_t{
  node_t *head[SLABNUM];
  lock_t  lock[SLABNUM];
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
    for(int j=0;j<SLABNUM;j++)lockInit(&slab[i].lock[j]);
    for(int j=0,blooksize=128;j<SLABNUM-1;j++,blooksize<<=1){
      slab[i].head[j]=pt;
      slab[i].head[j]->addr=pt;
      slab[i].head[j]->size=2*PAGESIZE;
      slab[i].head[j]->blockNum=2*PAGESIZE/blooksize;
      slab[i].head[j]->next=NULL;
      pt+=2*PAGESIZE;
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


static void *slab_alloc(size_t size){
  int slabOrder=targetList(size);
  int cpu=cpu_current();
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

static void memory_init(void *ptr){
  head=(header_t*)ptr;
  head->addr=ptr;
  head->size=heap.end-ptr;
  head->next=head->prev=NULL;
  printf("%x %x\n",head,head->size);
}

static void *kalloc(size_t size) {
  size=tableSizeFor(size);
  if(size<MINSIZE)size=MINSIZE;
  if(size>MAXSIZE)return NULL;
  else if(size>=MINSIZE&&size<=PAGESIZE)return slab_alloc(size);
  return NULL;
}

static void kfree(void *ptr){
  size_t size=1<<sizeOfPage[orderOfPage(ptr)];
  if(size<=PAGESIZE&&size>=MINSIZE)slab_free(ptr,size);
  printf("%d\n",size);
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  void *pt=heap.start;
  pt=slab_init(pt);
  memory_init(pt);
  void *fuck=kalloc(127);
  kfree(fuck);
  printf("%d\n",sizeof(struct node_t));
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