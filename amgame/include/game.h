#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

typedef struct Ball{
  int x,y;
}Ball;

void update();
void splash();
int print_key();
void move_ball(int direction);
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
