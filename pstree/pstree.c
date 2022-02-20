#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>

DIR *dir;

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  opendir("~");
  assert(dir!=NULL);
  return 0;
}
