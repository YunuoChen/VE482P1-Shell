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

#include "exe_funct.h"
#include "macro.h"
#include "parse.h"

static char cmd[MAX_COMMAND_LENGTH + 1];
// static char parse[MAX_COMMAND_LENGTH + 1];

// static bool quit_flag;
// static bool thread_flag = false;
static bool reading_flag;
// void execute(struct parse *parsed_cmd);

void stop_threads(int signo)
{
    // printf("catch you\n");
    if (signo == SIGINT)
    {
        // thread_flag = true;
        if (reading_flag == 1)
        {
            printf("\nmumsh $ ");
            fflush(stdout);
        }
        else
        {
            printf("\n");
            fflush(stdout);
        }
        // char out = '\n';
        // write(STDIN_FILENO,&out,1);
        // exit(0);
    }
    // printf("flage change to %d\n", thread_flag);
}



int main()
{
    // printf("test\n");

    /* PID manager initialization*/
    struct PID_manager PID_m;
    PID_m.childP_number = 0;  
    PID_m.finish_index = -1;

    /* Signal Ctrl+C*/
    signal(SIGINT, stop_threads);

    /* Quit_flage for exit and ctrl+D*/
    // bool quit_flag = false;    

    /* Store the standard input and output*/
    int inFds = dup(STDIN_FILENO);
    int outFds = dup(STDOUT_FILENO);

    /* Reading comman loop*/
    while (1)
    {
        reading_flag = true;

        /* Prompt*/
        fflush(stdin);
        printf("mumsh $ ");
        fflush(stdout);

        /* Read Loop*/
        

        bool ctn = false;
        char wtchr;
        int index = 0;
        
        if (Read_cmd(&wtchr, &ctn, cmd, &index) == 1)
        {
            prepare_quit(inFds, outFds, &PID_m);
            return 0;
        }
        
        /* Finish Reading*/
        /* Judge the status of quit reading*/
        if (ctn == 1)
        {
            printf("syntax error near unexpected token `%c'\n", wtchr);
            fflush(stdout);
            ctn = 0;
            continue;
        }
        /* Reading flag about the appear time of Ctrl + D*/
        reading_flag = false;
        /* if index == 0, read nothin*/
        if (index == 0) continue;

        /* temporary storage of cmd*/
        char *midT = (char*) malloc(strlen(cmd) + 1);
        strcpy(midT, cmd);
        bool back_flag = find_bk(cmd, index);

        /* if the command has a &*/
        if (back_flag == 1)
        {            
            /* store the corresponding cmd with & */
            PID_m.cmd_origin[PID_m.childP_number] = (char*) malloc(strlen(midT) + 1);
            strcpy(PID_m.cmd_origin[PID_m.childP_number], midT);
            free(midT);
            
            // PID_m.cmd_origin[PID_m.childP_number][index] = 0;

            /* Number ++*/
            PID_m.childP_number++;

            /*print background running hint*/
            printf("[%d] %s\n", PID_m.childP_number, PID_m.cmd_origin[PID_m.childP_number - 1]);
            fflush(stdout);
            
            /* Fork a thread */
            pid_t pid = fork();
            // PID_m.PIDs[PID_m.childP_number - 1] = getpid();
            if (pid == -1)
            {
                /* fail in forking a thread*/
                // int ret = -1;
            }

            /* The child process do*/
            else if (pid == 0)
            {
                parse_exe_all(outFds, inFds, cmd, &PID_m);
                exit(0);
            }
            /* The Father process do*/
            else
            {
                /* thread the pid of child process*/
                PID_m.PIDs[PID_m.childP_number - 1] = pid;
                continue;
            }
        }
        /* If the comman is a normal one without &*/
        else
        {
            free(midT);

            if (parse_exe_all(outFds, inFds, cmd, &PID_m) == 1)
            {
                return 0;
            }
            // fprintf(stderr, "tes\n\n\n\n");
        }
    }
    wait_all();
    return 0;
}
