#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define STACK_SIZE 16384
#define NAME_LENGTH 50
#define MAX_CO 128
#define stack_switch(x)\
     asm volatile(\
      "#if" "__x86_64__"\
      "movq %0, %%rsp"\
        ::"b"((uintptr_t)coInfo[x].rsp)\
      "#else"\
      "movl %%esp,%0"\
        ::"g"(coInfo[x].rsp)\
      "#endif"\
    )

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct coNode{
  struct co *addr;
  struct coNode *next;
};

struct co {
  char name[NAME_LENGTH];
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  int waitfor;
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
  void* sp;
  struct co *next,*prev;
};

struct co coInfo[MAX_CO];
static struct co *coHead=NULL,*coTail=NULL,*current=NULL;
static int coNum=1;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp-96),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

static inline struct co *coFind(int n){
  struct co *ans=coHead;
  for(int i=0;i<n;i++)ans=ans->next;
  return ans;
}

/*static inline void waitingsAdd(struct co *wait){
  struct coNode *temp;
  temp->addr=wait;
  if(current->waitings==NULL)temp->next=NULL;
  else temp->next=current->waitings->next;
  current->waitings=temp;
}*/

static void coFree(struct co *wasted){
  if(wasted->prev)wasted->prev->next=wasted->next;
  if(wasted->next)wasted->next->prev=wasted->prev;
  free(wasted);
  coNum--;
}


struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  coTail->next=malloc(sizeof(struct co));
  coTail->next->prev=coTail;
  coTail=coTail->next;
  struct co *ans=coTail;
  if(name)strcpy(ans->name,name);
  ans->arg=arg;
  ans->waitfor=0;
  ans->func=func;
  ans->status=CO_RUNNING;
  ans->waiter=NULL;
  ans->next=NULL;
  ans->sp=(void*)(ans->stack+sizeof(ans->stack));
  coNum++;
  if(!setjmp(current->context)){
    asm volatile(
      #if __x86_64__
      "movq %0, %%rsp"
      ::"b"((uintptr_t)ans->sp-8)
      #else
      "movl %0, %%esp"
      ::"b"((uintptr_t)ans->sp-8)
      #endif
      );
    if(!setjmp(ans->context)){
      longjmp(current->context,1);
    }
    else {
      ans->func(ans->arg);
    }
    ans->status=CO_DEAD;
    if(ans->waiter){
      struct co* wait=ans->waiter;
      ans->waiter=NULL;
      wait->waitfor--;
      assert(wait->waitfor>=0);
      if(!wait->waitfor)wait->status==CO_RUNNING;
    }
    co_yield();
  }

  return ans;
}

void co_wait(struct co *co) {
  co->waiter=current;
  current->status=CO_WAITING;
  current->waitfor++;
  while(co->status!=CO_DEAD){
    if(!setjmp(current->context))
    co_yield();
  }
  coFree(co);
}

void co_yield() {
  struct co* prev=current;
  do{
    current=coFind(rand()%coNum);
  }while(current->status==CO_DEAD||current->status==CO_WAITING);
  if(current==prev)return;
  if(!setjmp(prev->context)){
    if(current->status==CO_NEW){
      current->status=CO_RUNNING;
      asm volatile(
      #if __x86_64__
      "movq %0, %%rsp"
      ::"b"((uintptr_t)current->sp-8)
      #else
      "movl %0, %%esp"
      ::"b"((uintptr_t)current->sp-8)
      #endif
      );
      current->func(current->arg);
    }
    else if(current->status==CO_RUNNING){
      longjmp(current->context,1);
    }

    assert(current->status==CO_NEW||current->status==CO_RUNNING);
    //执行到这里说明该协程已经执行完毕
  }
  else{
  }
}

void first(){
    printf("fuck\n");
}

void second(){
  while(1){
   printf("shit\n");
    co_yield();
  }
}



void entry(void *arg) {
  while (1) {
    printf("%s", (const char *)arg);
    co_yield();
  }
}

/*int main() {
  struct co *co1 = co_start("co1", entry, "a");
  struct co *co2 = co_start("co2", entry, "b");
  co_wait(co1); // never returns
  co_wait(co2);
}*/

__attribute__((constructor))void initial(){
  srand((unsigned)time(NULL));
  coHead=malloc(sizeof(struct co));
  coHead->status=CO_RUNNING;
  coHead->next=coHead->prev=NULL;
  coHead->waiter=NULL;
  coHead->waitfor=0;
  strcpy(coHead->name,"main");
  coTail=current=coHead;
}