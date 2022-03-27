#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char command[100];
int main(int argc, char *argv[]) {
  char *exec_arg[] = {"strace","ls",  NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  /*char *exec_argv[argc+2];
  exec_argv[0]="strace",exec_argv[1]="-tt";
  for(int i=1;i<argc;i++)exec_argv[i+1]=argv[i];*/
  pid_t pid=fork();
  if(!pid){

  }
  execve("strace",          exec_arg, exec_envp);
  fprintf(stderr,"\n");
  execve("/bin/strace",     exec_arg, exec_envp);
  execve("/usr/bin/strace", exec_arg, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}