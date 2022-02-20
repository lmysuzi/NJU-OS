#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

#define PRO_NAME_LEN 30
#define PATH_NAME_LEN 40

typedef struct proc{
  pid_t pid,ppid;
  char pname[PRO_NAME_LEN];
}proc;

const char originPath[7]="/proc/";
const char targetFileName[6]="/stat";
DIR *dir=NULL;
struct dirent *dirent=NULL;
FILE *fp=NULL;
char path[PATH_NAME_LEN];

bool inline isNumber(char* s){
  while(*s!='\0'){
    if(*s<'0'||*s>'9')return false;
    s++;
  }
  return true;
}

void fileHandle(){
  memset(path,0,sizeof(path));
  strcpy(path,originPath);
  strcat(path,dirent->d_name);
  strcat(path,targetFileName);
  fp=fopen(path,"r");
  assert(fp!=NULL);
  int pid,ppid;
  char pname[PRO_NAME_LEN];
  char temp[9];
  fscanf(fp,"%d %s %s %d",&pid,pname,temp,&ppid);
  fclose(fp);
  printf("%d\n",pid);
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
      fileHandle();
    }
    dirent=readdir(dir);
  }
  
  return 0;
}
