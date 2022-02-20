#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdbool.h>

const char path[1000]="/proc";
DIR *dir;
struct dirent *fuck;

bool inline isNumber(char* s){
  while(*s!='\0'){
    if(*s<'0'||*s>'9')return false;
    s++;
  }
  return true;
}

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
    if(isNumber(fuck->d_name))
    printf("%s\n",fuck->d_name);
    fuck=readdir(dir);
  }
  
  return 0;
}
