#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "getpath.h"
#include "redirect.h"
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

void nopipe(Com *pcom,char *command,JOB *pjob) // no pipe child commands
{
  //create child process and make parent wait
  pid_t child_pid;
  int rpid = fork();//create child process
  int status = 0;
  if(rpid < 0)
    perror("fork failded");
  else if(rpid == 0)//child(new process)  
  {
    if(pcom->redirect == 1)//redirect but no pipe
    {
      one_program(command,pcom);
    }
    else // no redirect or pipe
    {
      if(execvp(pcom->expath,pcom->argv)== -1)
      {
        fprintf(stderr,"Error: invalid program\n");
        exit(errno);
      }
      
    }
  }
  else//parent process
  {
    child_pid = waitpid(-1,&status,WUNTRACED);
    // int k = WIFEXITED(status);
    // if(k == 1)
    // {
    //   fprintf(stderr,"Error: invalid program\n");
    // }
    int j = WIFSTOPPED(status);
    if(j)
    {
      add_job(child_pid,command,pjob);
    }
  }
}

void output_mix(char *out_symbol,Com* pcom) // < >> + < >
{
  char input[500] = {0};
  char output[500] = {0};
  int count = 0;
  int fd_input;
  int fd_output;

  for (int i = 0; i < pcom->argc; ++i)
  {
    if(!strcmp(pcom->argv[i],"<"))//strcmp如果相等，会返回0
    {
      strcpy(input, pcom->argv[i+1]);
      if(count == 0){
        if(i > count)
          count = i;
      }
    } 
    if(!strcmp(pcom->argv[i],out_symbol))//strcmp如果相等，会返回0
    {
      strcpy(output, pcom->argv[i+1]);
      if(count == 0){
        if(i > count)
          count = i;
      }
    }
  }
  pcom->argc = count;
  pcom->argv[count] = NULL;
  pcom->argv[count+1] = NULL;
  pcom->argv[count+2] = NULL;
  pcom->argv[count+3] = NULL;

  fd_input = open(input,O_RDONLY);
  if(!strcmp(out_symbol,">>")) // < >>
    fd_output = open(output,O_CREAT | O_APPEND | O_WRONLY,S_IRUSR | S_IWUSR);
  else // < >
    fd_output = open(output,O_CREAT | O_TRUNC | O_WRONLY,S_IRUSR | S_IWUSR);

  dup2(fd_input,0);
  dup2(fd_output,1);
  close(fd_input);
  close(fd_output);
  execvp(pcom->expath,pcom->argv); 
}

void single(char *inout_symbol,Com* pcom) // > + < + >>
{
  char filename[500];
  int fd;
  for (int i = 0; pcom->argv[i] != NULL; ++i)
  {
    if(!strcmp(pcom->argv[i],inout_symbol))//strcmp如果相等，会返回0
    {
      strcpy(filename, pcom->argv[i+1]);
      pcom->argv[i] = NULL;
      pcom->argv[i+1] = NULL;
      pcom->argc = i;
      break;
    } 
  }
  if(!strcmp(inout_symbol,"<")) // <
  {
    fd = open(filename,O_RDONLY);
    
    
      dup2(fd,0);
      close(fd);
      execvp(pcom->expath,pcom->argv); 
    
  }
  else if(!strcmp(inout_symbol,">>"))
  { // >>
    fd = open(filename,O_CREAT | O_APPEND | O_WRONLY,S_IRUSR | S_IWUSR);
    dup2(fd,1);
    close(fd);
    execvp(pcom->expath,pcom->argv); 
  }
  else
  { // >
    fd = open(filename,O_CREAT | O_TRUNC | O_WRONLY,S_IRUSR | S_IWUSR);
    dup2(fd,1);
    close(fd);
    execvp(pcom->expath,pcom->argv); 
  }
}

void one_program(char *command,Com *pcom) // no pipe, only redirect
{
  
 
  if(strrchr(command,'<'))
  {
    if(strstr(command,">>")) // < >>
    {
      //cat - input.txt < input.txt >> append2.txt
      // cat < input.txt >> output.txt
      // cat >> output.txt < input.txt
      char out_symbol[20] = {">>"};
      output_mix(out_symbol,pcom);
    }
    else if(strrchr(command,'>')) // < >
    { 
      // cat > output.txt < input.txt
      // cat < input.txt > output.txt
      // cat - input.txt > output4.txt < input.txt
      char out_symbol[20] = {">"};
      output_mix(out_symbol,pcom);
    }
    else // < read
    { // cat < input.txt
      // cat - input.txt < input.txt

      //先遍历argv,找到 < 和 < 后面的文件名
      //然后重定向之后，把 < 和 < 后面的文件名都删了
      char inout_symbol[20] = {"<"};
      single(inout_symbol,pcom);
    }
  }
  else if(strstr(command,">>"))// >>
  { // echo world >> append.txt
    // cat >> output.txt

    char inout_symbol[20] = {">>"};
    single(inout_symbol,pcom);  
  }
  else if(strrchr(command,'>'))// >
  { // cat > output.txt
    // echo -n hello > output2.txt
    // echo hello > output.txt
    char inout_symbol[20] = {">"};
    single(inout_symbol,pcom);
  }
  
}

int pipe_redirect(Com *pcom)
{
  //cat < input.txt | sort > output5.txt
    char filename[500];
    int fd;
    for (int i = 0; pcom->argv[i] != NULL; ++i)
    {
      if(!strcmp(pcom->argv[i],"<") || !strcmp(pcom->argv[i],">") || !strcmp(pcom->argv[i],">>"))//strcmp如果相等，会返回0
      {
        strcpy(filename, pcom->argv[i+1]);
        pcom->argv[i] = NULL;
        pcom->argv[i+1] = NULL;
        pcom->argc = i;
        break;
      } 
    }
    if(pcom->redirect == 1) //input <
    {
      fd = open(filename,O_RDONLY);
      
      
        dup2(fd,0);
      
    }
    else if(pcom->redirect == 3)
    { //append >>
      fd = open(filename,O_CREAT | O_APPEND | O_WRONLY,S_IRUSR | S_IWUSR);
      dup2(fd,1);
    }
    else
    { //output >
      fd = open(filename,O_CREAT | O_TRUNC | O_WRONLY,S_IRUSR | S_IWUSR);
      dup2(fd,1);
    }
    close(fd);
    return 0;
}

void pipe_single(Com *pcom) // one pipe
{ 
  // cat | cat 第一个写，第二个读
  // echo -e hello\nworld\nhello | sort
  pid_t apid, bpid;
  int status = 0;
  int pipefd[2] = {0};
  int i,j,k;

  pipe(pipefd);

  apid = fork();
  if(apid < 0)
    perror("fork a");

  if(apid == 0) //child a
  {
    /* Child a execvps cat to pipe */
    //get first argv
    pcom->redirect = 0;
    for (i = 0; i < pcom->argc; ++i)
    {
      if(!strcmp(pcom->argv[i],"|"))
      {
          pcom->argc = i;
          pcom->argv[i] = NULL;
          break;
      }
      if(!strcmp(pcom->argv[i],"<"))
      {
        pcom->redirect = 1;
      }
      // if(!strcmp(pcom->argv[i],">")) //first program cannot redirect output
      // {
      //   fprintf(stderr,"Error: invalid command\n");
      // }
    }
    pcom = get_expath(pcom);


    if(pcom->redirect > 0)
    {
      if(pipe_redirect(pcom) != -1) // invalid file
      {
        close(pipefd[0]);          /* Close unused read end */
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[1]);          /* Reader will see EOF */
        execvp(pcom->expath,pcom->argv);
        exit(errno);
      }
    }
    else
    {
      close(pipefd[0]);          /* Close unused read end */
      dup2(pipefd[1],STDOUT_FILENO);
      close(pipefd[1]);          /* Reader will see EOF */
      execvp(pcom->expath,pcom->argv);
      exit(errno);
    }
  }

  bpid = fork();
  if(bpid < 0)
    perror("fork b");

  // echo -e hello\nworld\nhello | sort -r
  // get second argv
  if(bpid == 0) //child b
  {
    pcom->redirect = 0;
    for (i = 0, j = 0,k = 0; i < pcom->argc; ++i)
    {
      if(!strcmp(pcom->argv[i],"|"))
        j = i+1;   
      if(j > 0 && pcom->argv[j] != NULL)
        strcpy(pcom->argv[k++],pcom->argv[j++]);
      if(!strcmp(pcom->argv[i],">"))
      {
        pcom->redirect = 2;
      }
      if(!strcmp(pcom->argv[i],">>"))
      {
        pcom->redirect = 3;
      }
      // if(!strcmp(pcom->argv[i],"<")) //last program cannot redirect input
      // {
      //   fprintf(stderr,"Error: invalid command\n");
      // }
    }
    pcom->argc = k;
    pcom->argv[k] = NULL;
    pcom = get_expath(pcom);

    if(pcom->redirect > 0)
    {
      pipe_redirect(pcom);
    }

    close(pipefd[1]); /* Close unused write end */
    dup2(pipefd[0],STDIN_FILENO);
    close(pipefd[0]);
    execvp(pcom->expath,pcom->argv);
    exit(errno);
  }

  close(pipefd[0]);          
  close(pipefd[1]); 
  for (int i = 0; i < 2; ++i)
  {
    waitpid(-1,&status,0);
    // int j = WIFEXITED(status);
    // if(j)
    //   fprintf(stderr,"Error: invalid program\n");
  }
}

void rewrite_argv(char *com, Com *pcom)
{
  //rewrite argv but not pro
  //rewrite command --- delete excessive spaces
  char command[1000] = {0};
  strcpy(command,com);
  if(command[0] == ' ')
  {
    if(command[strlen(command)-1] == ' ') // pipe 中间的命令，前后都有空格
    {
      size_t i;
      for (i = 0; i < strlen(command)-2; ++i)
      {
        command[i] = command[i+1];
      }
      command[i] = '\0';
      command[i+1] = '\0';
    }
    else //pipe末尾的命令，只有开头有空格
    {
      size_t i;
      for (i = 0; i < strlen(command)-1; ++i)
      {
        command[i] = command[i+1];
      }
      command[i] = '\0';
    }
  }
  else //pipe 开头的第一个命令，只有末尾有空格
  {
    command[strlen(command)-1] = '\0';
  }

  pcom->argc = 0;
  char s[4]=" ";
  char *space = (char *)malloc(sizeof(char)*1000);

  //rewrite argv and argc
  if( !strrchr(command,' ') )//没有空格 无参数
  { 
    strcpy(pcom->argv[0], command);
    pcom->argv[1] = NULL;
    pcom->argc = 1;
  }
  else //有空格 有参数
  {
    strcpy(space,command);
    space = strtok(space,s);
    pcom->argc = 0;
    int i = 0;
    while(space != NULL){
      strcpy(pcom->argv[i],space);
      i = i+1;
      pcom->argc += 1;
      space = strtok(NULL,s);
    }
    pcom->argv[i] = NULL;
  }

  pcom->redirect = 0;
  //如果space已经为NULL,那就没法给它赋值了

  free(space);

}

void Pipe_multiple(Com *pcom)
{
  //echo -e hello\nworld\nhello | sort | uniq
  // N processes, N-1 pipes, fork N times
  pid_t cpid;
  int status = 0;
  int i;
  int pro_num;
  int pipefd[10][2] = {0};

  for (i = 0; i < pcom->pro_num-1; ++i)
  {
    pipe(pipefd[i]); // pipe[0] pipe[1]
  }

  for (pro_num = 0; pro_num < pcom->pro_num; ++pro_num) //0,1,2
  {
    cpid = fork();
    if(cpid < 0)
      perror("fork c");

    if(cpid == 0) // child process
    {
      rewrite_argv(pcom->pro[pro_num],pcom); //rewrite argc and argv
    //在rewrite里写一下判断，只能第一个program<，最后一个program>或者>> (待做)

      if(pro_num == 0) //first program
      {
        for (int i = 0; i < pcom->argc; ++i)
        {
          if(!strcmp(pcom->argv[i],"<")){
            pcom->redirect = 1;
          }
        }
        pcom = get_expath(pcom);

        if(pcom->redirect > 0){
          pipe_redirect(pcom);
        }

        dup2(pipefd[pro_num][1],1);
        for (i = 0; i < pcom->pro_num-1; ++i)
        {
          close(pipefd[i][0]); 
          close(pipefd[i][1]); 
        }
        execvp(pcom->expath,pcom->argv);
        exit(errno);
      }
      else if(pro_num == (pcom->pro_num-1)) //last program
      {
        for (int i = 0; i < pcom->argc; ++i)
        {
          if(!strcmp(pcom->argv[i],">"))
          {
            pcom->redirect = 2;
          }
          if(!strcmp(pcom->argv[i],">>"))
          {
            pcom->redirect = 3;
          }
        }
        pcom = get_expath(pcom);

        if(pcom->redirect > 0){
          pipe_redirect(pcom);
        }
        
        dup2(pipefd[pro_num-1][0],STDIN_FILENO);

        for (i = 0; i < pcom->pro_num-1; ++i)
        {
          close(pipefd[i][0]); 
          close(pipefd[i][1]); 
        }

        execvp(pcom->expath,pcom->argv);
        exit(errno);
      }
        else // middle program
      {
        pcom = get_expath(pcom);
        
        dup2(pipefd[pro_num-1][0],STDIN_FILENO); //read former one
        dup2(pipefd[pro_num][1],1); //write latter one
        for (i = 0; i < pcom->pro_num-1; ++i)
        {
          close(pipefd[i][0]); 
          close(pipefd[i][1]); 
        }
        execvp(pcom->expath,pcom->argv);
        exit(errno);
      }
    }
  }

  //wait and close
  for (int i = 0; i < pcom->pro_num-1; ++i)
  {
      close(pipefd[i][0]);
      close(pipefd[i][1]);
  }

  for (int i = 0; i < pcom->pro_num; ++i)
  {
    waitpid(-1,&status,0);
    // int j = WIFEXITED(status);
    // if(j)
    //   fprintf(stderr,"Error: invalid program\n");
    // pid = wait(&status);
    // int j = WEXITSTATUS(status);
    // printf("child's pid = %d, exit status = %d\n",pid,j);
  }
}


