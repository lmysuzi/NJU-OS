#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char command[100];
int main(int argc, char *argv[]) {
 // char *exec_argv[] = { "strace", "ls", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *exec_argv[argc+2];
  exec_argv[0]="strace",exec_argv[1]="-tt";
  for(int i=1;i<argc;i++)exec_argv[i+1]=argv[i];
  exec_argv[2]=NULL;
  execve("strace",          exec_argv, exec_envp);
  execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}