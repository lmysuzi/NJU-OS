#ifndef __OS_H__
#define __OS_H__

#include <common.h>

typedef struct irq{
  int seq,event;
  handler_t handler;
  struct irq *next;
}irq_t;

#endif