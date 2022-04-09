#ifndef __KMT_H__
#define __KMT_H__

#include <common.h>

struct task{
  int a;
};

struct spinlock{
  bool status;
  int flag;
  const char *name;
};

struct semaphore{

};

#endif 