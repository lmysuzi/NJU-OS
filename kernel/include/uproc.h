#ifndef __UPROC_H__
#define __UPROC_H__

void pgfault(Event ev,Context *context);
int syscall(Context *context);

#endif