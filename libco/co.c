#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define STACK_SIZE 128*1024
#define NAME_LENGTH 128

enum co_status {
  CO_RUNNING=1, // 已经执行过
  CO_WAITING,
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  char name[NAME_LENGTH];
  char stack[STACK_SIZE] __attribute__ (( aligned(16) )); // 协程的堆栈
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg ;
  void* sp ;
  struct co *next,*prev ;
  struct co* waiter ;
  enum co_status status ;  // 协程的状态
  jmp_buf        context ; // 寄存器现场 (setjmp.h)
};

static struct co *coHead=NULL,*current=NULL,*coTail=NULL;
static int coNum=1;

static int flag=1;
static inline struct co *coFind(){
  if(flag){
    flag=0;
    struct co *ans=coHead;
    while(ans!=NULL){
      if(ans->status==CO_DEAD&&ans->waiter!=NULL)return ans->waiter;
      else if(ans->status==CO_RUNNING)return ans;
      ans=ans->next;
    }
  }
  else{
    flag=1;
    struct co *ans=coTail;
    while(ans!=NULL){
      if(ans->status==CO_DEAD&&ans->waiter!=NULL)return ans->waiter;
      else if(ans->status==CO_RUNNING)return ans;
      ans=ans->prev;
    }
  }
  return NULL;
}

static void coFree(struct co *wasted){
  if(wasted->prev)wasted->prev->next=wasted->next;
  if(wasted->next)wasted->next->prev=wasted->prev;
  if(wasted==coTail)coTail=coTail->prev;
  free(wasted);
  coNum--;
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  struct co *ans=malloc(sizeof(struct co));
  coTail->next=ans;
  ans->prev=coTail;
  ans->next=NULL;
  coTail=ans;
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
      current->status=CO_DEAD;
      co_yield();
    }
  }
  return ans;
}

void co_wait(struct co *co) {
  co->waiter=current;
  current->status=CO_WAITING;
  while(co->status!=CO_DEAD){
    if(!setjmp(current->context))co_yield();
  }
  current->status=CO_RUNNING;
  coFree(co);
}

void co_yield() {
  struct co* prev=current;
  current=coFind();
  assert(current!=NULL);
  if(!setjmp(prev->context)){
    longjmp(current->context,1);
  }
}

__attribute__((constructor))void initial(){
  coHead=malloc(sizeof(struct co));
  coHead->status=CO_RUNNING;
  coHead->waiter=NULL;
  coHead->next=coHead->prev=NULL;
  strcpy(coHead->name,"main");
  current=coTail=coHead;
}