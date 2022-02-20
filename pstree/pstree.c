#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

const char originPath[6]="/proc/";
const char targetFileName[5]="/stat";
DIR *dir=NULL;
struct dirent *dirent=NULL;
FILE *fp=NULL;
char path[40]="";

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
  dir=opendir(originPath);
  assert(dir!=NULL);
  dirent=readdir(dir);
  while (dirent!=NULL)
  {
    if(isNumber(dirent->d_name)){
      memset(path,0,sizeof(path));
      strcpy(path,originPath);
      strcat(path,dirent->d_name);
      strcat(path,targetFileName);
      printf("%s\n",path);
    }
    dirent=readdir(dir);
  }
  
  return 0;
}
