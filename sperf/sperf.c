#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char command[100];
int main(int argc, char *argv[]) {
  /*char *exec_argv[] = { "strace","-C", "ls", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  execve("strace",          exec_argv, exec_envp);
  execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);*/
  char a[6]="abc";
  printf("%p\n",a);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}