#ifndef __KMT_H__
#define __KMT_H__

#include <common.h>

#define KSTACK_SIZE 4096
#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)

enum{
  TASK_READY=1,TASK_RUNNING,TASK_SLEEP,TASK_READY_TO_WAKE,TASK_WAKED,TASK_DEAD,
};

void sleep_insert(task_t *task,uint64_t end_time);
task_t *task_now();
task_t *ucreate(task_t *task, const char *name,int pid);


#endif 