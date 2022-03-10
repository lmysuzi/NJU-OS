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
  CO_RUNNING, // 已经执行过
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct coDead{
  struct co *addr;
  struct coDead *next,*prev;
};

struct co {
  char name[NAME_LENGTH]__attribute__ (( aligned(16) ));
  char stack[STACK_SIZE] __attribute__ (( aligned(16) )); // 协程的堆栈
  void (*func)(void *)__attribute__ (( aligned(16) )); // co_start 指定的入口地址和参数
  void *arg __attribute__ (( aligned(16) ));
  void* sp __attribute__ (( aligned(16) ));
  struct co *next,*prev __attribute__ (( aligned(16) ));
  struct co* waiter __attribute__ (( aligned(16) ));
  enum co_status status __attribute__ (( aligned(16) ));  // 协程的状态
  jmp_buf        context __attribute__ (( aligned(16) )); // 寄存器现场 (setjmp.h)
};

static struct co *coHead=NULL,*current=NULL;
static struct coDead *deadHead=NULL;
static int coNum=1;

static inline struct co *coFind(int n){
  struct co *ans=coHead;
  for(int i=0;i<n;i++)ans=ans->next;
  return ans;
}

static inline void deadDelete(struct coDead* deleted){
  if(deleted->prev)deleted->prev->next=deleted->next;
  if(deleted->next)deleted->next->prev=deleted->prev;
  free(deleted);
}

static inline struct co *deadFind(){
  struct coDead *temp=deadHead;
  while(temp!=NULL){
    if(temp->addr->waiter!=NULL){
      struct co *ans=temp->addr->waiter;
      deadDelete(temp);
      printf("fuck\n");
      return ans;
    }
    else{
      struct coDead* t=temp;
      temp=temp->next;
      deadDelete(t);
    }
  }
  return NULL;
}

static inline struct co *deadReturn(){
  struct co *ans=coHead;
  while(ans!=NULL){
    if(ans->status==CO_DEAD&&ans->waiter!=NULL)return ans->waiter;
    ans=ans->next;
  }
  return NULL;
}

static inline void deadAdd(struct co *added){
  struct coDead *ans=malloc(sizeof(struct coDead));
  ans->addr=added;
  ans->prev=NULL;
  if(deadHead!=NULL){
    ans->next=deadHead->next;
    deadHead->prev=ans;
  }
  else ans->next=NULL;
  deadHead=ans;
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
    current->status=CO_DEAD;
    //deadAdd(current);
    co_yield();
    }
  }
  return ans;
}

void co_wait(struct co *co) {
  co->waiter=current;
  while(co->status!=CO_DEAD){
    if(!setjmp(current->context))co_yield();
  }
  coFree(co);
}

void co_yield() {
  struct co* prev=current;
  current=deadReturn();
  if(current==NULL){
    if(coNum==1)current=coHead;
    else{
      current=coHead->next;
      while(current!=NULL){
        printf("fukc\n");
        if(current->status!=CO_DEAD)break;
        current=current->next;
      }
      if(current==NULL)current=coHead;
    }
  }
  if(!setjmp(prev->context)){
    longjmp(current->context,1);
  }
}

__attribute__((constructor))void initial(){
  srand((unsigned)time(NULL));
  coHead=malloc(sizeof(struct co));
  coHead->status=CO_RUNNING;
  coHead->waiter=NULL;
  coHead->next=coHead->prev=NULL;
  strcpy(coHead->name,"main");
  current=coHead;
}