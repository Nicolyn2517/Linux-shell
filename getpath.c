#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "getpath.h"
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

char cwd[PATH_MAX+1];


// typedef struct
// {
//   int job_id[110]; //job index
//   pid_t job_pid[110]; //job pid
//   char **command_list; //job command
//   int job_sum; // total number of jobs
// } JOB;


void add_job(pid_t child_pid, char *command,JOB *pjob)
{
  int i = pjob->job_sum;

  pjob->job_id[i] = i + 1; // job_id
  pjob->job_id[i+1] = 0; // end

  pjob->job_pid[i] = child_pid;// job_pid
  pjob->job_pid[i+1] = 0; // end

  strcpy(pjob->command_list[i],command);// command

  pjob->job_sum += 1;

  // printf("add job id = %d\n",pjob->job_id[i]);
  // printf("add job pid = %d\n",pjob->job_pid[i]);
  // printf("add job command = %s\n",pjob->command_list[i]);
  // printf("add job sum = %d\n",pjob->job_sum);
}

void build_in(Com* pcom, char *baseName,JOB *pjob)
{
  int flag = 1;
  for (int i = 0; i < pcom->argc; ++i)
  {
    if(!strcmp(pcom->argv[i],"<") || !strcmp(pcom->argv[i],">") || !strcmp(pcom->argv[i],"|")) //Built-in commands (e.g., cd) cannot be I/O redirected or piped.
    {
      fprintf(stderr,"Error: invalid command\n");
      flag = 0;
    }
  }
  if(flag == 1) // no error
  {
    if(!strcmp(pcom->argv[0],"cd"))
    {
      if(pcom->argv[1] == NULL || pcom->argv[2] != NULL)
      {
        fprintf(stderr,"Error: invalid command\n");
      }
      else
      {
        if(chdir(pcom->argv[1])!=0) //failed
        {
          fprintf(stderr,"Error: invalid directory\n");
        }
        else //success
        {
          //getcwd and prompt
          cwd_ppt(baseName);
        }
      }
    }
    else if(!strcmp(pcom->argv[0],"pwd")) //pwd 自己实现的功能便于测试的
    {
      printf("%s\n",cwd);
    }
    /*-----------exit--------------*/
    else if(!strcmp(pcom->argv[0],"exit")) //exit 
    {
      if(pjob->job_sum != 0)
      {
        fprintf(stderr, "Error: there are suspended jobs\n");
      }
      else if(pcom->argv[1] != NULL)
      {
        fprintf(stderr, "Error: invalid command\n");
        return;
      }
      else
      {
        exit(0);
      }
    }
    else if(!strcmp(pcom->argv[0],"jobs")) //jobs 
    {
      if(pcom->argv[1]!=NULL)
      {
        fprintf(stderr, "Error: invalid command\n");
      }
      else
      {
        output_job(pjob);
      }
    }
    else if(!strcmp(pcom->argv[0],"fg")) //fg 
    {
      int i;
      int status = 0;
      int jid = pcom->argv[1][0] - '0' - 1; // job id

      if(pcom->argv[1]==NULL || pcom->argv[2] != NULL)
      {
        fprintf(stderr, "Error: invalid command\n");
      }
      else if(pjob->job_sum <  (jid + 1))
      {
        fprintf(stderr, "Error: invalid job\n");
      }
      else
      {
        
        char *up_command = (char *)malloc(sizeof(char)*1000);
        pid_t up_pid;

        // printf("jid = %d\n",jid);
        // printf("sending signal to child %d:\n",pjob->job_pid[jid]);
        kill(pjob->job_pid[jid],SIGCONT);
        strcpy(up_command,pjob->command_list[jid]);
        up_pid = pjob->job_pid[jid];
        //update
        pjob->job_sum = pjob->job_sum - 1;
        for (i = jid; pjob->job_pid[i+1] != 0; ++i)
        {
          strcpy(pjob->command_list[i],pjob->command_list[i+1]);
          pjob->job_pid[i] = pjob->job_pid[i+1];
        }
        pjob->job_pid[i] = 0;
        pjob->job_id[pjob->job_sum] = 0;

        pid_t child_pid = waitpid(up_pid,&status,WUNTRACED);
        int j = WIFSTOPPED(status);
        if(j)
        {
          // printf("adding child %d\n",child_pid);
          if(child_pid == up_pid)
            add_job(child_pid,up_command,pjob);
        }
        free(up_command);
        }
    }
  }
}


void output_job(JOB *pjob)
{
  for (int i = 0; i < pjob->job_sum; ++i)
  {
    printf("[%d] %s\n",pjob->job_id[i],pjob->command_list[i]);
  }
}

void handler_ignore()
{
  
}

void handlerZ()
{
  // printf("Z!!!!\n");
}

void cwd_ppt(char *baseName)
{
  if (getcwd(cwd, sizeof(cwd)) != NULL) 
  {
    baseName = base_Name(cwd,baseName);
  }
  else 
  {
   perror("getcwd() error");
  }
}




char *base_Name(char cwd[],char *basePath){

  char baseSlash[256];//带/的base路径
  char frontSlash[256]={"[nyush "};//拼接的前一部分
  char backSlash[25]={"]$ "};//拼接的后一部分

  strcpy(baseSlash, strrchr(cwd,'/'));//得到带/的base路径

  if(strlen(baseSlash) > 1)//"/2250" = 5
  {
    size_t i;
    for ( i = 0; i < strlen(baseSlash)-1; ++i)//0,1,2,3
    {
      baseSlash[i] = baseSlash[i+1];
    }
    baseSlash[i] = '\0';
    strcpy(basePath,baseSlash);
  }
  else//"/"root directory
  {
    strcpy(basePath,"/");
  }
  strcat(frontSlash,basePath);

  strcpy(basePath , frontSlash);

  strcpy(basePath , strcat(basePath,backSlash));

  return basePath;
}

// typedef struct
// {
//   int argc;
//   char **argv;
// } Com;

// valid commands:
// A blank line. 空行！
// /usr/bin/ls -a -l
// cat shell.c | grep main | less
// cat < input.txt （cat input.txt）
// cat > output.txt
// cat >> output.txt
// cat < input.txt > output.txt
// cat < input.txt >> output.txt
// cat > output.txt < input.txt
// cat >> output.txt < input.txt
// cat < input.txt | cat > output.txt
// cat < input.txt | cat | cat >> output.txt
// echo world >> append.txt
// cat - input.txt < input.txt >> append2.txt

// Here are some examples of invalid commands:

// cat <
// cat >
// cat |
// | cat
// cat << file.txt
// cat < file.txt < file2.txt
// cat < file.txt file2.txt
// cat > file.txt > file2.txt
// cat > file.txt >> file2.txt
// cat > file.txt file2.txt
// cat > file.txt | cat
// cat | cat < file.txt
// cd / > file.txt
Com *get_expath(Com *pcom){

  char path2[1000] = "./";
  char path3[1000] = "/usr/bin/";

  if(pcom->argc > 0)
  {
    if(pcom->argv[0][0] == '/')
    {//absolute path
      strcpy(pcom->expath,pcom->argv[0]);
    }
    else if(strrchr(pcom->argv[0],'/'))
    {//relative path
      strcpy(pcom->expath,strcat(path2,pcom->argv[0]));
    }
    else
    {//base name
      if((strcmp(pcom->argv[0],"cd")) && (strcmp(pcom->argv[0],"exit")) && (strcmp(pcom->argv[0],"pwd")) && (strcmp(pcom->argv[0],"jobs")) && (strcmp(pcom->argv[0],"fg")))
        //不是cd exit pwd
        strcpy(pcom->expath,strcat(path3,pcom->argv[0]));
    }
  }
  return pcom;
}

Com *save_command(char *command){

  if(strlen(command) > 1)
      command[strlen(command)-1] = '\0';

  Com *pcom = (Com *)malloc(sizeof(Com));//Com指针
  if(pcom == NULL){
    printf("pcom malloc failed\n");
  }

  //first: get argc
  pcom->argc = 0;
  char s[4]=" ";
  char *space = (char *)malloc(sizeof(char)*1000);

  //strtok如果第一次找不到对应字符，会直接返回目标字符串的指针，它返回null只是因为到了字符串末尾

//get argc and argv

  if(strrchr(command,' ')){//有空格 得出argc的值

    strcpy(space,command);
    pcom->argc = 1;
    space = strtok(space,s);
    while(strtok(NULL,s)){
      pcom->argc += 1;
    }
  }
  else{//没有空格 
    // 1.blank line 2.只有命令没有参数（ls） 3.乱码 （需要考虑这种情况吗，待询问）（需要，待做）

    if(!strcmp(command,"\n")){//只有一个回车 blank line
      pcom->argc = 0;
    }
    else{
      pcom->argc = 1;
    }
  }

  //second: malloc argv
  pcom->argv = (char **)malloc(sizeof(char*)*(pcom->argc+1));
  for(int i = 0; i < pcom->argc; i++){//0,1,2,3 暂时没给null分配扩展空间，看debug情况
    //事实证明null不需要扩展的二维空间，一维就够了
    pcom->argv[i] = (char*)malloc((sizeof(char))*800); 
  }

  //third: write argv
  if(pcom->argc == 1){
    strcpy(pcom->argv[0], command);
    pcom->argv[1] = NULL;
  }
  else if(pcom->argc > 1){
    strcpy(space,command);
    space = strtok(space,s);
    int i = 0;
    while(space != NULL){
      strcpy(pcom->argv[i++],space);
      space = strtok(NULL,s);
    }
    pcom->argv[i] = NULL;
  }
  //如果space已经为NULL,那就没法给它赋值了
free(space);

//get pro, pro_num and redirect
  if(pcom->argc > 0)//not blank line
  {
    if(strrchr(command,'|')){//有pipe | 不止一个program
      // cat < input.txt | cat | cat >> output.txt
      //先确定program的个数
      pcom->pro_num = 1;
      for (size_t i = 0; i < strlen(command); ++i)
      {
        if(command[i] == '|')
          pcom->pro_num += 1;
      }
    }
    else
    { //无pipe | 只有一个program
      pcom->pro_num = 1;
    }
  }
  else
  {  // blank line
    pcom->pro_num = 0;
  }

  if(pcom->argc > 0) // not blank line
  {
    //malloc pro
    pcom->pro = (char **)malloc(sizeof(char*)*(pcom->pro_num+1)); //最后一位是null
    for(int i = 0; i < pcom->pro_num; i++)
    {
      pcom->pro[i] = (char*)malloc((sizeof(char))*800); 
    }
    
    //write pro
    // cat < input.txt | cat | cat >> output.txt
    space = (char *)malloc(sizeof(char)*1000);
    strcpy(s,"|");
    strcpy(space,command);
    
    space = strtok(space,s);
    int i = 0;
    while(space != NULL){
      strcpy(pcom->pro[i++],space);
      space = strtok(NULL,s);
    }
    pcom->pro[i] = NULL;
    free(space);

    //write redirect
    if(strrchr(command,'<') || strrchr(command,'>')  || pcom->pro_num > 1) //have redirection symbols
    {
      pcom->redirect = 1;
    }
    else
      pcom->redirect = 0;

     if(strstr(command,"<<")) //cat << file.txt
    {
      fprintf(stderr,"Error: invalid command\n");
      pcom->argc = 0;
      pcom->pro_num = 0;
      pcom->redirect = 0;
      return pcom;
    }
    // printf("111\n");
    if(!strcmp(pcom->argv[0],"|") ) // | cat
    {
      // printf("| at the beginning\n");
      fprintf(stderr,"Error: invalid command\n");
      pcom->argc = 0;
      pcom->pro_num = 0;
    }
    // printf("222\n");
    if(pcom->pro_num > 1) // 有 pipe  
    {
      if(pcom->pro[pcom->pro_num-1] == NULL)
      {
        // printf("no argument after | \n");
        fprintf(stderr,"Error: invalid command\n"); // cat |
        pcom->argc = 0;
        pcom->pro_num = 0;
      }

      // for (int i = 0; i < pcom->argc; ++i)
      // {
      //   if(!strcmp(pcom->argv[i],"<") || !strcmp(pcom->argv[i],">") || !strcmp(pcom->argv[i],">>"))
      //   {
      //     fprintf(stderr,"Error: invalid command\n"); // cat |
      //     pcom->argc = 0;
      //     pcom->pro_num = 0;
      //   }
      // }
    }
    // printf("333\n");
    if(pcom->pro_num>1)// 有 pipe  
    {
        if(strrchr(pcom->pro[0],'>'))
        {
          // printf("the first program to >\n");
          fprintf(stderr,"Error: invalid command\n"); // cat |
          pcom->argc = 0;
          pcom->pro_num = 0;
          return pcom;
        }
    }
    // printf("444\n");
    if(pcom->pro_num > 1)
    {
        if(strrchr(pcom->pro[pcom->pro_num-1],'<'))
        {
          // printf("the last program to <\n");
          fprintf(stderr,"Error: invalid command\n"); // cat |
          pcom->argc = 0;
          pcom->pro_num = 0;
          return pcom;
        }
    }

    if(pcom->argc > 0 && pcom->pro_num > 0 && pcom->redirect !=0)
    {
      for (int i = 0; i < pcom->argc; ++i)
      {
        if(!strcmp(pcom->argv[i],"<")) // <
        {
          if(pcom->argv[i+1] == NULL) // cat <
          {
            fprintf(stderr,"Error: invalid command\n");
            pcom->argc = 0;
            pcom->pro_num = 0;
            pcom->redirect = 0;
            return pcom;
          }
          else if(pcom->argv[i+2]!=NULL) // != | 
          {
            if(!strcmp(pcom->argv[i+2],"<")) // <+< 
            {
              fprintf(stderr,"Error: invalid command\n");
              pcom->argc = 0;
              pcom->pro_num = 0;
              pcom->redirect = 0;
              return pcom;
            }
            if(strcmp(pcom->argv[i+2],">")) // != >
            {
              if(strcmp(pcom->argv[i+2],">>")) // != >>
              {
                if(strcmp(pcom->argv[i+2],"|")) // < .txt .txt
                {
                  fprintf(stderr,"Error: invalid command\n");
                  pcom->argc = 0;
                  pcom->pro_num = 0;
                  pcom->redirect = 0;
                  return pcom;
                }
              }
            }
          }  
        }
        else if(!strcmp(pcom->argv[i],">"))
        {
          if(pcom->argv[i+1] == NULL) // cat >
          {
            fprintf(stderr,"Error: invalid command\n");
            pcom->argc = 0;
            pcom->pro_num = 0;
            pcom->redirect = 0;
            return pcom;
          }
          else if(pcom->argv[i+2]!=NULL) // != | 
          {
              if(!strcmp(pcom->argv[i+2],">>")) // > + >>
            {
              fprintf(stderr,"Error: invalid command\n");
              pcom->argc = 0;
              pcom->pro_num = 0;
              pcom->redirect = 0;
              return pcom;
            }
              if(!strcmp(pcom->argv[i+2],">")) // > + >
            {
              fprintf(stderr,"Error: invalid command\n");
              pcom->argc = 0;
              pcom->pro_num = 0;
              pcom->redirect = 0;
              return pcom;
            }
            if(strcmp(pcom->argv[i+2],"<")) // != >
            {
              if(strcmp(pcom->argv[i+2],"|")) // > .txt .txt
              {
                fprintf(stderr,"Error: invalid command\n");
                pcom->argc = 0;
                pcom->pro_num = 0;
                pcom->redirect = 0;
                return pcom;
              }
            }
          }
        }
        else if(!strcmp(pcom->argv[i],">>"))
        {
          if(pcom->argv[i+1] == NULL) // cat >>
          {
            fprintf(stderr,"Error: invalid command\n");
            pcom->argc = 0;
            pcom->pro_num = 0;
            pcom->redirect = 0;
            return pcom;
          }
        }

      }
    }

    if(pcom->argc > 0 && pcom->pro_num > 0 && pcom->redirect !=0)
    {
      for (int i = 0; i < pcom->argc; ++i)
      {
        if(!strcmp(pcom->argv[i],"<"))
        {
          int fd = open(pcom->argv[i+1],O_RDONLY);
          if(fd == -1)
          {
            fprintf(stderr,"Error: invalid file\n");
            pcom->argc = 0;
            pcom->pro_num = 0;
            pcom->redirect = 0;
            return pcom;
          }
          close(fd);
          
          
        }
      }
    }

  int flag1 = 0;
  int flag2 = 0;
  
    for (int i = 0; i < pcom->argc; ++i)
    {
      if(!strcmp(pcom->argv[i],">"))
      {
        flag1 = 1;
      }
      if(!strcmp(pcom->argv[i],">>"))
      {
        flag2 = 1;
      }
    }
    if(flag1 && flag2)
    {
      fprintf(stderr,"Error: invalid command\n");
      pcom->argc = 0;
            pcom->pro_num = 0;
            pcom->redirect = 0;
            return pcom;
    }
   


  }
  return pcom;
}


