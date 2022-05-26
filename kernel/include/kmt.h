#ifndef __KMT_H__
#define __KMT_H__

#include <common.h>

#define KSTACK_SIZE 4096

void sleep_insert(task_t *task,uint64_t end_time);
task_t *task_now();


#endif 