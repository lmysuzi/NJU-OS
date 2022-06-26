#ifndef __OS_H__
#define __OS_H__

#include <common.h>

typedef struct irq{
  int seq,event;
  handler_t handler;
  struct irq *next;
}irq_t;

struct task{
  int id,pid;
  int child_count;
  int status;
  int np;
  void *va[60],*pa[60];
  const char *name;
  AddrSpace as;
  Context *context;
  struct task *next,*prev;
  uint8_t *kstack;
};

struct spinlock{
  bool status;
  int flag;
  const char *name;
};

typedef struct sem_tasks{
  task_t *task;
  struct sem_tasks *prev,*next;
}sem_tasks_t;

struct semaphore{
  spinlock_t lock;
  int count;
  const char *name;
  sem_tasks_t *sem_tasks;
};

typedef struct sleep_tasks{
  task_t *task;
  uint64_t time;
  struct sleep_tasks *prev,*next;
}sleep_tasks_t;
#endif
