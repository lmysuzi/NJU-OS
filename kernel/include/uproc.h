#ifndef __UPROC_H__
#define __UPROC_H__


int syscall(Context *context);
Context* pgfault(Event ev,Context *context);
#endif