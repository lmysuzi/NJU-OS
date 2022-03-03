#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>

#define STACK_SIZE 8192

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
  void* rsp;
};

struct co *current=NULL;
struct co cosn[3];
int num=1;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
 // cos[num].name=name;
  cosn[num].func=func;
  cosn[num].arg=arg;
  cosn[num].status=CO_NEW;
  cosn[num].waiter=NULL;
  return &cosn[num++];
}

void co_wait(struct co *co) {
}

void co_yield() {
  setjmp(current->context);
  int chosen=rand()/num;
  current=&cosn[chosen];
  longjmp(cosn[chosen].context,chosen);
}

void first(){
  while(1){
    printf("fuck\n");
    co_yield();
  }
}

void second(){
  while(1){
   printf("shit\n");
    co_yield();
  }
}



int main()
{
  setjmp(cosn[0].context);
  while(1){
  co_start(NULL,first,NULL);
  co_start(NULL,second,NULL);
  co_yield();
  }
  return 0;
}

__attribute__((constructor))void initial(){
  cosn[0].status=CO_RUNNING;
  cosn[0].func=(void*)main;
  cosn[0].rsp=cosn[0].stack+sizeof(cosn[0].stack);
  
  asm volatile(
    
    #if __x86_64__
    "movq %0, %%rsp; jmp *%1"
      : : "b"((uintptr_t)cosn[0].rsp),     "d"(cosn[0].func)
#else
    "movl %0, %%esp; jmp *%1"
      : : "b"((uintptr_t)cosn[0].rsp - 8), "d"(cosn[0].func)
#endif
  );
  current=&cosn[0];
}