#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
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
      fopen("/tmp/crepl_temp/fuck.txt","r");


    }
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}
