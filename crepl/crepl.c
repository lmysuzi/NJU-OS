#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

char *myArgv[]={
  "gcc",
  #if __X86_64__
  "-m64",
  #else
  "-m32",
  #endif
  "-w","-xc","-fPIC","-shared","-o",NULL,NULL,NULL
};

static int file_num=0;

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
      char path[100]="/tmp/crepl_temp/";
      char filename[100];
      memset(filename,0,100*sizeof(char));
      sprintf(filename,"file%d",file_num);
      strcat(path,filename);
      char c_path[100],so_path[100];
      memset(c_path,0,100*sizeof(char));
      memset(so_path,0,100*sizeof(char));
      sprintf(c_path,"%s",path);
      sprintf(so_path,"%s",path);
      strcat(c_path,".c");
      strcat(so_path,".so");
      printf("%s\n",c_path);
      printf("%s\n",so_path);
      FILE *fp=fopen(path,"w+");
      assert(fp!=NULL);
      
    }
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}
