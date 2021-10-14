
#ifndef PARSE_H
#define PARSE_H
#include "macro.h"




// typedef enum {
//     ECHO, 
//     CAY,
//     APTGET, 
//     LS, 
//     EXIT, 
// } Linux_CMD;

// typedef enum {
//     WRITELN, 
//     CTRL, 
//     WAIT
// } cmd_mode;


struct parse
{
    ERROR_TYPE error_flag;
    char error_msg[2];
    int cmd_num;
    char *cmd;
    char *argv[MAX_COMMAND_LENGTH];
    char *redirect_input;
    char *redirect_output;
    char *cd_direct;
    bool is_dual;
    bool re_input;
    bool re_output;
    bool background_flag;
    bool cd_flag;
    bool pwd_flag;
};

void parsed_init(struct parse *parsed_cmd);

int pre_cut(char *cmd, struct parse *parsed_cmd);

void my_split(char *cmd, struct parse *parsed_cmd);
void FREE_ALL(struct parse *parsed_cmd);
void parsed_fresh(struct parse *parsed_cmd);
#endif//PARSE_H
