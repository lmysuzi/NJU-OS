#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  mkdir("/tmp/crepl_temp",S_IRWXU|S_IRWXG|S_IRWXO);
  static char line[4096];

  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if(strncmp(line,"int",3)==0){
      char name[]="fuck.cXXXXXX";
      int file=mkstemp(name);
      printf("%d\n",file);
      unlink(name);
      close(file);
      fopen("/tmp/crepl_temp/fuck.txt","w");


    }
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}
