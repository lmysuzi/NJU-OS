#include <game.h>

// Operating system is a C program!



int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();

  puts("Press any key to see its key code...\n");
  while (1) {
    int code=print_key();
    if(code==1)halt(0);
    move_ball(code);
    update();
  }
  return 0;
}
