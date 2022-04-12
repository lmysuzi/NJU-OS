#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

typedef struct sysCall{
  double time;
  char name[100];
}sysCall;
sysCall syscalls[1000];
int sysNum=0;
double totalTime=0;
int second=0;
char buf[1024];
char name[1024];
char time[1024];
char str[1000];
char *exec_argv[1024];
struct timeval prev,now;

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
  if(sysNum>=1000)return;
  strcpy(syscalls[sysNum].name,name);
  syscalls[sysNum].time=time;
  sysNum++;
}

void draw(){
  qsort(syscalls,sysNum,sizeof(sysCall),cmp);
  printf("\033[34m==========================================\n");
  printf("\033[32mTime: %ds\n",second);
  for(int i=0;i<sysNum&&i<6;i++){
    printf("\033[31m%s",syscalls[i].name);
    printf("\033[33m(%d%%)\n",(int)(syscalls[i].time*100/totalTime));
  }
  for(int i=0;i<80;i++)printf("%c",'\0');
}
int main(int argc, char *argv[]) {
  extern char **environ;
  //char *exec_envp[] = { "PATH=/bin", NULL, };
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
    for(;end<strlen(path);end++){
      if(path[end]==':'){
        memset(str,0,1000);
        if(end<begin)continue;
        strncpy(str,path+begin,end-begin);
        str[end-begin]='\0';
        strcat(str,"/strace");
        begin=end+1;
        execve(str,exec_argv,environ);
      }
      else if(end==strlen(path)-1){
        memset(str,0,1000);
        if(end<=begin)continue;
        strncpy(str,path+begin,end-begin+1);
        str[end-begin+1]='\0';
        strcat(str,"/strace");
        execve(str,exec_argv,environ);
      }
    }
  }
  else{
    close(pipefd[1]);
    FILE *fp=fdopen(pipefd[0],"r");
    gettimeofday(&prev,NULL);
    gettimeofday(&now,NULL);
    memset(buf,0,1024);
    while(1){
      if(fgets(buf,1024,fp)==NULL)break;
      if(buf[0]=='\0')goto fuck;
      if(buf[0]<'a'||buf[0]>'z')continue;
      if(strlen(buf)<=2)continue;
      if(buf[strlen(buf)-2]!='>')continue;
      int t;
      for(t=0;t<strlen(buf);++t){
        if(buf[t]=='(')break;
      }
      if(t==strlen(buf))continue;
      strncpy(name,buf,t);
      name[t]='\0';
      for(t=strlen(buf)-2;t>=0;--t){
        if(buf[t]=='<')break;
        else if((buf[t]<'0'||buf[t]>'9')&&buf[t]!='.')goto fuck;
      }
      if(t<0)continue;
      double timeNum;
      sscanf(buf+t+1,"%lf",&timeNum);
      update(name,timeNum);
      totalTime+=timeNum;
      fuck:
      gettimeofday(&now,NULL);
      if(now.tv_sec!=prev.tv_sec){
        second++;
        prev=now;
        draw();
      }
      memset(buf,0,1024);
    }
    draw();
    return 0;
  } 
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
