#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

const char path[1000]="/proc";
DIR *dir;
struct dirent *fuck;

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  dir=opendir(path);
  assert(dir!=NULL);
  fuck=readdir(dir);
  while (fuck!=NULL)
  {
    printf("%s\n",fuck->d_name);
    fuck=readdir(dir);
  }
  
  return 0;
}
