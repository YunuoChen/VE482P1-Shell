#ifndef EXE_H
#define EXE_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <signal.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h>   
#include <sys/stat.h>
#include <pwd.h>

#include "macro.h"
#include "parse.h"


void my_cout(char *str, char *pre_str, char *back_str);
void error_print(ERROR_TYPE ERR, char *msg);
int open_dir(char *direct);
int cd_dir(char *direct);
void prepare_quit(const int inFds, const int outFds, struct PID_manager *PID_m);
void my_pwd();
bool allspace(struct parse *parsed_cmd);
bool is_missing(struct parse *parsed_cmd, int partition_number);
void wait_all();
bool Read_cmd(char *wtchr, bool *ctn, char *cmd, int * index);
int parse_exe_all(const int outFds, const int inFds, char* cmd, struct PID_manager *PID_m);
bool find_bk(char* cmd, int index);
bool allspace(struct parse *parsed_cmd);
void wait_mine(pid_t *PW, int total);
void print_my_jobs(struct PID_manager *pm);
int call_Redi(struct parse *parsed_cmd, int total, int index, const int outFDS);

#endif//EXE_H
