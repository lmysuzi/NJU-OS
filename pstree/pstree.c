#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>

#define PROC_NAME_LEN 50
#define PATH_NAME_LEN 40
#define MAX_PROC_NUM 4096
#define MAX_SUBPROC 70
#define TAB "       "
#define PRINT_TABS(X) for(int i=0;i<X;i++)printf(TAB)
#define PRINT_PID(X) printf("(%d)",X)
#define originPath "/proc/"
#define targetFileName "/stat"
#define showPids "--show-pids"
#define numericSort "--numeric-sort"
#define version "--version"
#define PRINT_VERSION fprintf(stderr,Version)
#define Version "\
This is a pstree devoted by Li Mingyang in 2022\n\
\n\
pstree (PSmisc) UNKNOWN\n\
Copyright (C) 1993-2019 Werner Almesberger and Craig Small\n\
\n\
PSmisc comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome to redistribute it under\n\
the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n"

typedef struct proc{
  pid_t pid,ppid;
  char pname[PROC_NAME_LEN];
}proc;

typedef struct proOrder{
  int order,pid;
}proOrder;
int comp(const void* a,const void* b){return (*(proOrder*)a).pid>(*(proOrder*)b).pid;}

proc procs[MAX_PROC_NUM];
char path[PATH_NAME_LEN];
struct dirent *dirent=NULL;
DIR *dir=NULL;
FILE *fp=NULL;
int procNum=0;
bool _show_pids,_numeric_sort,_version,redirect;

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
  printf("%s",procs[now].pname);
  if(_show_pids)PRINT_PID(procs[now].pid);
  printf("\n");
  if(_numeric_sort){
    int subnum=0;
    proOrder temp[MAX_SUBPROC];
    for(now=0;now<procNum;now++){
      if(procs[now].ppid==ppid){
        temp[subnum].order=now;
        temp[subnum++].pid=procs[now].pid;
      }
    }
    qsort(temp,subnum,sizeof(proOrder),comp);
    for(int i=0;i<subnum;i++){
      printTree(temp[i].pid,level+1,temp[i].order);
    }
    return;
  }
  for(now=0;now<procNum;now++){
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
    PRINT_VERSION;return 0;
  }
  assert(!argv[argc]);
  dir=opendir(originPath);
  assert(dir!=NULL);
  dirent=readdir(dir);
  while (dirent!=NULL){
    if(isNumber(dirent->d_name))fileHandle();
    dirent=readdir(dir);
  }
  printTree(1,0,0);
  return 0;
}
