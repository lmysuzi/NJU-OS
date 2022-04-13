#ifndef __KMT_H__
#define __KMT_H__

#include <common.h>

#define KSTACK_SIZE 4000

struct task{
  int status;
  const char *name;
  Context *context;
  struct task *next;
  uint8_t kstack[KSTACK_SIZE];
};

struct spinlock{
  bool status;
  int flag;
  const char *name;
};

struct semaphore{

};

#endif 