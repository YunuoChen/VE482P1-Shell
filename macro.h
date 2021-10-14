#ifndef MACRO_H
#define MACRO_H

#define MAX_COMMAND_LENGTH 1024

#define bool char
#define true 1
#define false 0

typedef enum{
    IDLE,
    REDIRECT_IN,
    REDIRECT_OUT,
    ARGV,
    IN_FILENAME,
    OUT_FILENAME,
    CD_R,
    SINGLE_LEFT,
    DOUBLE_LEFT
} read_mode;


typedef enum{
    NO_ERROR,
    NONEXIST_PROGRM,
    NONEXIST_FILE,
    FAIL_OPEN,
    DUPLI_IN,
    DUPLI_OUT,
    SYNTAX_ERROR,
    MISSING,
    CD_NONE,
    PWD,
    EXIT,
    CD_SIGNAL
} ERROR_TYPE;

struct PID_manager
{
    int childP_number, finish_index;
    pid_t PIDs[MAX_COMMAND_LENGTH];
    // int P_state[MAX_COMMAND_LENGTH];
    char *cmd_origin[MAX_COMMAND_LENGTH];
};

// typedef enum {
//     TERMINATE,
//     CONTINUE,
//     READING
// } READING_STATE;

#endif //MACRO_H
