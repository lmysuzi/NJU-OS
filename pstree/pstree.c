#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

const char path[1000]="/proc";
DIR *dir;
struct dirent *fuck;

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  dir=opendir("/proc");
  assert(dir!=NULL);
  fuck=readdir(dir);
  while (fuck!=NULL)
  {
    if(isdigit("2"))
    printf("%s\n",fuck->d_name);
    fuck=readdir(dir);
  }
  
  return 0;
}
