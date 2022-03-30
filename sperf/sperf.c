#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

char *timePattern="[<][0-9]+[.][0-9][>]";

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
    regex_t reg;
    regcomp(&reg,timePattern,0);
    regmatch_t pos;
    while(fgets(buf,4096,fp)!=NULL){
      regexec(&reg,buf,4096,&pos,0);
      printf("%d",pos.rm_so);
    }
    return 0;
  } 
  perror(argv[0]);
  exit(EXIT_FAILURE);
}

  //execve("strace",          exec_argv, exec_envp);
  //execve("/bin/strace",     exec_argv, exec_envp);