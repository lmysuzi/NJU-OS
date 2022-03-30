#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

typedef struct sysCall{
  double time;
  char name[20];
}sysCall;
sysCall syscalls[100];

int main(int argc, char *argv[]) {
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *exec_argv[]={"strace","-T","ls",">","/dev/null",NULL};
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
    execve("/bin/strace",     exec_argv, exec_envp);
  }
  else{
    close(pipefd[1]);
    char buf[4096];
    FILE *fp=fdopen(pipefd[0],"r");
    while(fgets(buf,4096,fp)!=NULL){
      if(buf[0]<'a'||buf[0]>'z')continue;
      if(buf[strlen(buf)-2]!='>')continue;
      char name[20];
      int i=0;
      while(buf[i]!='('){
        name[i]=buf[i];i++;
      }
      name[i]='\0';
      printf("%s",buf);
      printf("%s\n",name);
    }
    return 0;
  } 
  perror(argv[0]);
  exit(EXIT_FAILURE);
}

  //execve("strace",          exec_argv, exec_envp);
  //execve("/bin/strace",     exec_argv, exec_envp);