#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "getpath.h"
#include <fcntl.h>
#include "redirect.h"
#include <errno.h>
#include <signal.h>

//reference
//https://www.cnblogs.com/jeakon/archive/2012/05/26/2816828.html
// https://love.junzimu.com/archives/1003
//https://www.runoob.com/cprogramming/c-structures.html 
//https://blog.csdn.net/tong_xin2010/article/details/41895507
//https://blog.csdn.net/qq_25908839/article/details/102733282
// https://cloud.tencent.com/developer/article/2105673
// https://stackoverflow.com/questions/23998283/variable-warning-set-but-not-used
//https://www.runoob.com/cprogramming/c-switch.html
//https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
// https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus
// https://linuxize.com/post/linux-cat-command/
// https://www.youtube.com/watch?v=PIb2aShU_H4&ab_channel=KrisJordan
// https://blog.csdn.net/Roland_Sun/article/details/32084825

extern char cwd[PATH_MAX+1];//当前目录的绝对路径


//int argc, const char *const *argv
int main() {
  //rewrite signal
  signal(SIGINT,handler_ignore); // ignore Ctrl - C
  signal(SIGTSTP,handlerZ); // ignore Ctrl - Z
  signal(SIGQUIT,handler_ignore); //ignore Ctrl - quit

  //prepare
  memset(cwd, 0, PATH_MAX+1);
  size_t bufsize = 1000;//命令不超过1000个字符
  char *command = (char *)malloc(bufsize * sizeof(char));//命令行输入的命令
  char *baseName = (char *)malloc(sizeof(char)*(PATH_MAX+1));//不带/的base路径
  JOB *pjob = (JOB *)malloc(sizeof(JOB));
  pjob->command_list = (char **)malloc(sizeof(char*)*110);
  for (int i = 0; i < 110; ++i)
  {
    pjob->command_list[i] = (char *)malloc(sizeof(char)*1000);
  }
  pjob->job_sum = 0;
  Com *pcom = NULL;

  //getcwd and prompt
  cwd_ppt(baseName);

  while(1){

    //print prompt
    printf("%s",baseName);
    fflush(stdout);
    //read command
    if(getline(&command,&bufsize,stdin)==EOF){
      exit(0);
    }
    //save command
    pcom = save_command(command);
    //get expath
    pcom = get_expath(pcom);
    //run program
    if(pcom->argc > 0) // not blank line
    {
      // printf("pcom->redirect = %d\n",pcom->redirect);
      if((!strcmp(pcom->argv[0],"cd")) || (!strcmp(pcom->argv[0],"pwd")) || (!strcmp(pcom->argv[0],"exit")) || (!strcmp(pcom->argv[0],"jobs")) || (!strcmp(pcom->argv[0],"fg"))) 
      {
        build_in(pcom,baseName,pjob);//build-in commands
      }
      else if(pcom ->pro_num == 2)
      {
        // printf("pipe_single\n");
        pipe_single(pcom);//single pipe
      }
      else if(pcom->pro_num > 2) 
      {
        // printf("Pipe_multiple\n");
        Pipe_multiple(pcom);//multiple pipe
      }
      else 
      {

        nopipe(pcom,command,pjob);//child commands(no pipe)
      }
    }
      fflush(stdin);
      fflush(stdout);
      // free(pcom);//待做，free结构体指针以及里面的二维数组指针
  }

  free(baseName);
  free(command);
  return 0;
}
