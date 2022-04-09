#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

typedef struct sysCall{
  double time;
  char name[20];
}sysCall;
sysCall syscalls[100];
int sysNum=0;
double totalTime=0;
int second=0;

int cmp(const void *a,const void *b){
  return ((sysCall*)b)->time>((sysCall*)a)->time?1:-1;
}

void update(char *name,double time){
  if(!sysNum){
    strcpy(syscalls[0].name,name);
    syscalls[0].time=time;
    sysNum++;
    return;
  }
  for(int i=0;i<sysNum;i++){
    if(strcmp(syscalls[i].name,name)==0){
      syscalls[i].time+=time;return;
    }
  }
  assert(sysNum<100);
  strcpy(syscalls[sysNum].name,name);
  syscalls[sysNum].time=time;
  sysNum++;
}

void draw(){
  qsort(syscalls,sysNum,sizeof(sysCall),cmp);
  printf("\033[34m==========================================\n");
  printf("\033[32mTime: %ds\n",second);
  for(int i=0;i<sysNum;i++){
    printf("\033[31m%s",syscalls[i].name);
    printf("\033[33m(%d%%)\n",(int)(syscalls[i].time*100/totalTime));
  }
  for(int i=0;i<80;i++)printf("%c",'\0');
}
int main(int argc, char *argv[]) {
  char *a=getenv("PATH");
  printf("%d\n",strlen(a));
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *exec_argv[argc+4];
  exec_argv[0]="strace",exec_argv[1]="-T";
  for(int i=1;i<argc;i++){
    exec_argv[i+1]=argv[i];
  }
  exec_argv[argc+1]=">",exec_argv[argc+2]="/dev/null",exec_argv[argc+3]=NULL;
  int pipefd[2];
  if(pipe(pipefd)==-1){
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  pid_t pid=fork();
  if(pid<0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid==0){
    close(pipefd[0]);
    dup2(pipefd[1],STDERR_FILENO);
    close(STDOUT_FILENO);
    char *path=getenv("PATH");int begin=0,end=0;
    while(1){

    }
    execve("/usr/bin/strace",     exec_argv, exec_envp);
    execve("/bin/strace",     exec_argv, exec_envp);
    execve("/sbin/strace",     exec_argv, exec_envp);
    execve("/usr/sbin/strace",     exec_argv, exec_envp);
  }
  else{
    close(pipefd[1]);
    char buf[4096];
    FILE *fp=fdopen(pipefd[0],"r");
    struct timeval prev,now;
    gettimeofday(&prev,NULL);
    while(fgets(buf,4096,fp)!=NULL){
      if(buf[0]<'a'||buf[0]>'z')continue;
      if(buf[strlen(buf)-2]!='>')continue;
      char name[20];
      int i=0;
      while(buf[i]!='('){
        name[i]=buf[i];i++;
      }
      name[i]='\0';
      i=strlen(buf)-2;
      char time[20];
      while(buf[i]!='<')i--;
      i++;
      int j=0;
      while(buf[i]!='>'){
        time[j]=buf[i];i++;j++;
      }
      time[j]='\0';
      double timeNum;
      sscanf(time,"%lf",&timeNum);
      update(name,timeNum);
      totalTime+=timeNum;
      gettimeofday(&now,NULL);
      if(now.tv_sec!=prev.tv_sec){
        second++;
        prev=now;
        draw();
      }
    }
    draw();
    return 0;
  } 
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
