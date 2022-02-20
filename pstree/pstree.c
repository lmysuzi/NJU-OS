#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

#define PROC_NAME_LEN 50
#define PATH_NAME_LEN 40
#define MAX_PROC_NUM 1000
#define TAB "       "
#define PRINT_TABS(X) for(int i=0;i<X;i++)printf(TAB)
#define originPath "/proc/"
#define targetFileName "/stat"
#define showPids "--show-pids"
#define numericSort "--numeric-sort"
#define version "--version"
#define Version "This is a pstree devoted by Li Mingyang in 2022\
pstree (PSmisc) UNKNOWN\n\
Copyright (C) 1993-2019 Werner Almesberger and Craig Small\n\
\n\
PSmisc comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome to redistribute it under\n\
the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n"
#define print_version printf(Version);

typedef struct proc{
  pid_t pid,ppid;
  char pname[PROC_NAME_LEN];
}proc;

proc procs[MAX_PROC_NUM];
char path[PATH_NAME_LEN];
struct dirent *dirent=NULL;
DIR *dir=NULL;
FILE *fp=NULL;
int procNum=0;
bool _show_pids,_numeric_sort,_version;

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
  char pname[PROC_NAME_LEN];
  char temp[9];
  fscanf(fp,"%d %s %s %d",&pid,pname,temp,&ppid);
  fclose(fp);
  procs[procNum].pid=pid;
  procs[procNum].ppid=ppid;
  strcpy(procs[procNum].pname,pname+1);
  procs[procNum].pname[strlen(procs[procNum].pname)-1]='\0';
  procNum++;
}

void printTree(int ppid,int level,int now){
  PRINT_TABS(level);
  printf("%s\n",procs[now].pname);
  for(;now<procNum;now++){
    if(procs[now].ppid==ppid){
      printTree(procs[now].pid,level+1,now);
    }
  }
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    if(i==0)continue;
    assert(argv[i]);
    if(strcmp(argv[i],"-p")==0||strcmp(argv[i],showPids)==0)_show_pids=1;
    else if(strcmp(argv[i],"-n")==0||strcmp(argv[i],numericSort)==0)_numeric_sort=1;
    else if(strcmp(argv[i],"-V")==0||strcmp(argv[i],version)==0)_version=1;
    else printf("Unknown arg: %s\n",argv[i]);
  }
  if(_version){
    print_version;
    return 0;
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
  printTree(1,0,0);
  return 0;
}
