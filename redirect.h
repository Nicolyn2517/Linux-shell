#ifndef _REDIRECT_H_
#define _REDIRECT_H_

void one_program(char *command,Com *pcom);
void pipe_single(Com *pcom);
int pipe_redirect(Com *pcom);
void single(char *inout_symbol,Com* pcom);
void output_mix(char *out_symbol,Com* pcom);
void Pipe_multiple(Com *pcom);
void rewrite_argv(char *command,Com *pcom);
void nopipe(Com *pcom,char *command,JOB *pjob);
#endif