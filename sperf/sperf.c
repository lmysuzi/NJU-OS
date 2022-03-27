#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char command[100];
int main(int argc, char *argv[]) {
  char *exec_argv[] = {"strace","ls",  NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *exec_argv[argc+2];
  exec_argv[0]="strace",exec_argv[1]="-tt";
  for(int i=1;i<argc;i++)exec_argv[i+1]=argv[i];
  pid_t pid=fork();
  if(!pid){
    execve("/bin/strace",exec_argv,exec_envp);
  }
  else{

  }
 // perror(argv[0]);
  //exit(EXIT_FAILURE);
}