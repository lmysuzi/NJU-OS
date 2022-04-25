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
      FILE *fp=fdopen(file,"w");
      fwrite("fuck",1,4,fp);
      char buf[100];
      fseek(fp,0,SEEK_SET);
      fread(buf,1,4,fp);
      printf("%s\n",buf);
      fclose(fp);


    }
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}
