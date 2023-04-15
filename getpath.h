#ifndef _GETPATH_H_
#define _GETPATH_H_

typedef struct
{
	int argc;
	int pro_num;
	int redirect;
	char **argv;
	char **pro;
	char expath[1000];
} Com;

typedef struct
{
	int job_id[110]; //job index
	pid_t job_pid[110]; //job pid
	char **command_list; //job command
	int job_sum; // total number of jobs
} JOB;

void add_job(pid_t child_pid, char *command,JOB *pjob);
void handler_ignore();
void output_job(JOB *pjob);
void handlerZ();
char *base_Name(char cwd[],char *basePath); //get base name from current path
Com* save_command(char *command); //divide command into argv and argc by spaces
void cwd_ppt(char *baseName); //get new cwd(current path) and base name
Com* get_expath(Com *pcom); // get execvp's path
// Com* deletere(Com *pcom,char symbol); //delete the symbol of redirect
void build_in(Com* pcom, char *baseName,JOB *pjob); // build-in commands

#endif