#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define STACK_SIZE 32*1024
#define NAME_LENGTH 64 

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  char name[NAME_LENGTH]__attribute__ (( aligned(16) ));
  char stack[STACK_SIZE] __attribute__ (( aligned(16) )); // 协程的堆栈
  void (*func)(void *)__attribute__ (( aligned(16) )); // co_start 指定的入口地址和参数
  void *arg __attribute__ (( aligned(16) ));
  void* sp __attribute__ (( aligned(16) ));
  struct co *next,*prev __attribute__ (( aligned(16) ));

  enum co_status status __attribute__ (( aligned(16) ));  // 协程的状态
  struct co *    waiter  __attribute__ (( aligned(16) ));  // 是否有其他协程在等待当前协程
  jmp_buf        context __attribute__ (( aligned(16) )); // 寄存器现场 (setjmp.h)
};

static struct co *coHead=NULL,*current=NULL;
static int coNum=1;

static inline struct co *coFind(int n){
  struct co *ans=coHead;
  for(int i=0;i<n;i++)ans=ans->next;
  return ans;
}

static void coFree(struct co *wasted){
  if(wasted->prev)wasted->prev->next=wasted->next;
  if(wasted->next)wasted->next->prev=wasted->prev;
  free(wasted);
  coNum--;
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  struct co *ans=malloc(sizeof(struct co));
  if(coHead->next)coHead->next->prev=ans;
  ans->next=coHead->next;
  coHead->next=ans;
  ans->prev=coHead;
  if(name)strcpy(ans->name,name);
  ans->arg=arg;
  ans->func=func;
  ans->status=CO_RUNNING;
  ans->waiter=NULL;
  ans->sp=(void*)(ans->stack+sizeof(ans->stack));
  coNum++;
  if(!setjmp(current->context)){
    
    if(!setjmp(ans->context)){
      longjmp(current->context,1);
    }
    else {
      asm volatile(
        #if __x86_64__
        "movq %0, %%rsp"
        ::"b"((uintptr_t)current->sp)
        #else
        "movl %0, %%esp"
        ::"b"((uintptr_t)current->sp)
        #endif
      );
      current->func(current->arg);
    }
    printf("\nwww\n");
    current->status=CO_DEAD;
    if(current->waiter!=NULL){
      current->waiter->status=CO_RUNNING;
    }
    co_yield();
  }
  return ans;
}

void co_wait(struct co *co) {
  if(co->status!=CO_DEAD){
  co->waiter=current;
  current->status=CO_WAITING;
  while(co->status!=CO_DEAD){
    if(!setjmp(current->context))co_yield();
  }
  }
  coFree(co);
}

void co_yield() {
  struct co* prev=current;
  do{
    current=coFind(rand()%coNum);
  }while(current->status==CO_WAITING||current->status==CO_DEAD);
  if(current==prev)return;
  if(!setjmp(prev->context)){
    longjmp(current->context,1);
  }
}

__attribute__((constructor))void initial(){
  srand((unsigned)time(NULL));
  coHead=malloc(sizeof(struct co));
  coHead->status=CO_RUNNING;
  coHead->next=coHead->prev=NULL;
  coHead->waiter=NULL;
  strcpy(coHead->name,"main");
  current=coHead;
}