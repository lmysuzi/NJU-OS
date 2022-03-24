#include <common.h>

#define PAGENUM  32000
#define PAGESIZE (4096)
#define MINSIZE  (128)
#define MAXSIZE  (16<<20)
#define MAXCPU 8

#define orderOfPage(x) (((uint64_t)(x-0x300000))>>12)

enum{
  _128=1,_256,_512,_1024,_2048,_4096,_2p,_4p,_8p,_16p,_32p,_64p,_128p,_256p,_512p,_1024p,_2048p,_4096p
};

//static uint8_t sizeOfPage[PAGENUM];

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

typedef struct node_t{
  void *addr;
  int blockNum;
  size_t size;
  struct node_t *next;
}node_t;

typedef struct slab_t{
  node_t *head128,*head256,*head512,*head1024,*head2048,*headpage;
}slab_t;

static slab_t slab[MAXCPU];

static void slab_init(void *pt){
  for(int i=0,n=cpu_count();i<n;i++){
    slab[i].head128=pt;
    slab[i].head128->addr=pt;
    slab[i].head128->size=2*PAGESIZE;
    slab[i].head128->blockNum=slab[i].head128->size/128;
    slab[i].head128->next=NULL;
    pt+=2*PAGESIZE;
    slab[i].head256=pt;
    slab[i].head256->addr=pt;
    slab[i].head256->size=2*PAGESIZE;
    slab[i].head256->blockNum=slab[i].head256->size/256;
    slab[i].head256->next=NULL;
    pt+=2*PAGESIZE;
    slab[i].head512=pt;
    slab[i].head512->addr=pt;
    slab[i].head512->size=2*PAGESIZE;
    slab[i].head512->blockNum=slab[i].head512->size/512;
    slab[i].head512->next=NULL;
    pt+=2*PAGESIZE;
    slab[i].head1024=pt;
    slab[i].head1024->addr=pt;
    slab[i].head1024->size=2*PAGESIZE;
    slab[i].head1024->blockNum=slab[i].head1024->size/1024;
    slab[i].head1024->next=NULL;
    pt+=2*PAGESIZE;
    slab[i].head2048=pt;
    slab[i].head2048->addr=pt;
    slab[i].head2048->size=2*PAGESIZE;
    slab[i].head2048->blockNum=slab[i].head2048->size/2048;
    slab[i].head2048->next=NULL;
    pt+=2*PAGESIZE;
    slab[i].headpage=pt;
    slab[i].headpage->addr=pt;
    slab[i].headpage->size=1000*PAGESIZE;
    slab[i].headpage->blockNum=1000;
    pt+=1000*PAGESIZE;
  }
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
  void *pt=heap.start;
  printf("%d\n",pmsize/PAGESIZE);
  slab_init(pt);
  for(int i=0;i<cpu_count();i++){
    printf("%x %x %x %x %x %x\n",slab[i].head128,slab[i].head256,slab[i].head512,slab[i].head1024,slab[i].head2048,slab[i].headpage);
    printf("%d %d %d %d %d %d\n",slab[i].head128->blockNum,slab[i].head256->blockNum,slab[i].head512->blockNum,slab[i].head1024->blockNum,slab[i].head2048->blockNum,slab[i].headpage->blockNum);
  }
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};