#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <dlfcn.h>

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
static int wrapper_num=0;
static char expresion[4096];

int main(int argc, char *argv[],char *env[]) {
  mkdir("/tmp/crepl_temp",S_IRWXU|S_IRWXG|S_IRWXO);
  static char line[4096];

  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }

    char path[100]="/tmp/crepl_temp/";
    char filename[100];
    memset(filename,0,100*sizeof(char));
    sprintf(filename,"file%d",file_num++);
    strcat(path,filename);
    char c_path[100],so_path[100];
    memset(c_path,0,100*sizeof(char));
    memset(so_path,0,100*sizeof(char));
    sprintf(c_path,"%s",path);
    sprintf(so_path,"%s",path);
    strcat(c_path,".c");
    strcat(so_path,".so");
    FILE *fp=fopen(c_path,"w+");
    myArgv[7]=so_path;
    myArgv[8]=c_path;
    
    if(strncmp(line,"int",3)==0){
      fwrite(line,1,strlen(line),fp);
      assert(fp!=NULL);
      fclose(fp);
      if(fork()==0){
        close(STDERR_FILENO);
        execve("/usr/bin/gcc",myArgv,env);
      }
      else{
        wait(NULL);
        void *handle=dlopen(so_path,RTLD_NOW);
        assert(handle!=NULL);
        int (*func)(void)=dlsym(handle,"a");
        printf("%d\n",func());
      }
    }
    else{
      memset(expresion,0,4096*sizeof(char));
      char func_name[20];
      sprintf(func_name,"wrapper%d()",wrapper_num++);
      printf("%s\n",func_name);
      fwrite(func_name,1,strlen(func_name),fp);
      strcat(expresion,"\n{return ");
      strcat(expresion,line);
      strcat(expresion,";}");
      printf("%s\n",expresion);
    }
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}
