#include "exe_funct.h"
static char buf[MAX_COMMAND_LENGTH + 1];

void my_cout(char *str, char *pre_str, char *back_str)
{
    printf("%s%s%s\n", pre_str, str, back_str);
    fflush(stdout);
}

bool allspace(struct parse *parsed_cmd)
{
    for (int i = 0; i < (int) strlen(parsed_cmd->cmd); i++)
    {
        if (!isspace(parsed_cmd->cmd[i]))
        {
            return false;
        }
    }
    return true;
}

void print_my_jobs(struct PID_manager *pm)
{
    // fprintf(stderr, "in exe %d: %d %d\n", pm->childP_number, pm->PIDs[0], pm->PIDs[1]);
    // fflush(stderr);
    for (int i = 0; i < pm->childP_number; i++)
    {
        int status;
        
        int wtpd = waitpid(pm->PIDs[i], &status, WNOHANG);
        // int wtpd = 0;
        // printf("%d %d\n",pm->PIDs[i], wtpd);
        if (wtpd == 0)
        {
            printf("[%d] running %s\n", i + 1, pm->cmd_origin[i]);
        }
        else
        {
            printf("[%d] done %s\n", i + 1, pm->cmd_origin[i]);
        }
        // if (wtpd == -1)
        // {
        //     printf("[%d] done %s\n", i + 1, pm->cmd_origin[i]);
            
        // }
        // else
        // {
        //     printf("[%d] running %s\n", i + 1, pm->cmd_origin[i]);
        // }
        fflush(stdout);
    }
}

int call_Redi(struct parse *parsed_cmd, int total, int index, const int outFDS)
{
    // fprintf(stderr,"\n\n in call redi \n\n\n");
    // fflush(stderr);
    int ret = 0;
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("call_redi Error");
        ret = -1;
    }
    else if (pid == 0)
    {
        if (parsed_cmd->re_input == 1)
        {
            // if (parsed_cmd->redirect_input == NULL)
            // {
            //     fprintf(stderr, "command %s [option]\n", "<");
            //     fflush(stderr);
            //     exit(-1);
            // }
            // printf("re in : %s\n", parsed_cmd.redirect_input);
            int fd = open(parsed_cmd->redirect_input, O_RDWR, 0664);
            if(fd == -1)
            {
                dup2(outFDS, STDOUT_FILENO);
                // if (errno == 30) printf("%s: Permission denied", parsed_cmd->redirect_input);
                printf("%s: %s\n",parsed_cmd->redirect_input, strerror(errno));
                fflush(stdout);
                exit(-1);
            }
            dup2(fd, 0);
            close(fd);
        }
        if (parsed_cmd->re_output == 1)
        {
            // fprintf(stderr,"re out : %s\n", parsed_cmd->redirect_output);
            fflush(stderr);
            if (!parsed_cmd->is_dual)
            {
                if (parsed_cmd->redirect_output == NULL)
                {
                    printf("command %s [option]\n", ">");
                    exit(-1);
                }
                int fd =open(parsed_cmd->redirect_output, O_RDWR|O_CREAT|O_TRUNC, 0664);
                if(fd == -1)
                {
                    dup2(outFDS, STDOUT_FILENO);
                    // printf("errno is %d", errno);
                    printf("%s: Permission denied\n", parsed_cmd->redirect_output);
                    // printf("%s: %s\n", parsed_cmd->redirect_output, strerror(errno));
                    fflush(stdout);
                    exit(-1);
                }
                dup2(fd, 1);
                close(fd);
            }
            else
            {
                if (parsed_cmd->redirect_output == NULL)
                {
                    // printf("command %s [option]", ">>");
                    // fflush(stdout);
                    exit(-1);
                }
                int fd =open(parsed_cmd->redirect_output, O_RDWR|O_CREAT|O_APPEND, 0664);
                if(fd == -1)
                {
                    // printf("out 2 open!\n");
                    // fflush(stdout);
                    exit(-1);
                }
                dup2(fd, 1);
                close(fd);
            }
        }
        else if (parsed_cmd->re_output == 0 && index == total - 1)
        {
            // fprintf(stderr,"redire %s\nnumber %d, %d", parsed_cmd->argv[0], index, total);
            dup2(outFDS, STDOUT_FILENO);
        }
        // fprintf(stderr, "cmd_num is %d", parsed_cmd->job_flag);
        // fflush(stderr);
        if (parsed_cmd->cmd_num > 0)
        {
            // fprintf(stderr,"before exe %s\n", parsed_cmd->argv[0]);
            // if (parsed_cmd->argv[0][0] == '<' || parsed_cmd->argv[0][0] == '>' ||parsed_cmd->argv[0][0] == '|')
            // {
            //     dup2(outFDS, STDOUT_FILENO);
            //     // printf("%s\n", strerror(errno));
            //     printf("error: missing program");
            //     fflush(stdout);
            //     exit(-1);
            // }
            execvp(parsed_cmd->argv[0], parsed_cmd->argv);
            // perror("fork\n");
            dup2(outFDS, STDOUT_FILENO);
            // printf("%s\n", strerror(errno));
            printf("%s: command not found\n", parsed_cmd->argv[0]);
            fflush(stdout);
            exit(-1);
        }
        if (parsed_cmd->pwd_flag == 1)
        {
            if (parsed_cmd->re_output == 0 && index == total - 1) dup2(outFDS, STDOUT_FILENO);
            my_pwd();
            exit(0);
        }
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        int child = WEXITSTATUS(status);
        if (child)
        {
            printf("%s\n",strerror(child));
        }
    }
    return ret;
}

int exeWithPipe(struct parse *parsed_cmd, int total, int index, const int outFDS, struct PID_manager *pm, pid_t *P_T_wait)
{
    // fprintf(stderr, "in exe %d: \n", pm->PIDs[0]);
    // fflush(stderr);
    // fprintf(stderr, "enyter is %d %d", index, total);
    // fflush(stderr);
    if (index == total) return 0;
    if (index == 0)
    {
        parsed_fresh(parsed_cmd);
        my_split(parsed_cmd->cmd, parsed_cmd);
        if (parsed_cmd->cd_flag == 1)
        {
            dup2(outFDS, STDOUT_FILENO);
            cd_dir(parsed_cmd->cd_direct);
            return 0;
        }
        if (parsed_cmd->argv[0] != NULL && strcmp(parsed_cmd->argv[0], "exit") == 0)
        {
            dup2(outFDS, STDOUT_FILENO);
            // quit_flag = true;
            return EXIT;
        }
    }
    if (index > 0)
    {
        parsed_fresh(parsed_cmd);
        my_split(parsed_cmd->cmd, parsed_cmd);
    }
    // for (int k = 0; k< parsed_cmd->cmd_num; k++)
    // {
    //     fprintf(stderr,"cmd %d is :%s\n", k, parsed_cmd->argv[k]);
    // }

    // fprintf(stderr,"out > is: %s\n isdual : %d\n", parsed_cmd->redirect_output, parsed_cmd->is_dual);
    // fprintf(stderr,"in < is: %s\n", parsed_cmd->redirect_input);
    // fprintf(stderr,"out is %d\n", parsed_cmd->re_output);
    if (parsed_cmd->error_flag != NO_ERROR)
    {
        dup2(outFDS, STDOUT_FILENO);
        // printf("error\n");
        error_print(parsed_cmd->error_flag, parsed_cmd->error_msg);
        return -1;
    }
    if (index > 0 && parsed_cmd->re_input == 1)
    {
        dup2(outFDS, STDOUT_FILENO);
        printf("error: duplicated input redirection\n");
        fflush(stdout);
        return -1;
    }
    if (index < total - 1 && parsed_cmd->re_output == 1)
    {
        dup2(outFDS, STDOUT_FILENO);
        printf("error: duplicated output redirection\n");
        fflush(stdout);
        return -1;
    }
    if (parsed_cmd->cmd_num == 0 && parsed_cmd->cd_flag == 0 && parsed_cmd->pwd_flag == 0)
    {
        dup2(outFDS, STDOUT_FILENO);
        printf("error: missing program\n");
        fflush(stdout);
        return -1;
    }
    // char *j = "jobs";
    // printf("%s\n%d\n", parsed_cmd->argv[0],strcmp(parsed_cmd->argv[0], j) == 1);
    if (parsed_cmd->argv[0] != NULL && strcmp(parsed_cmd->argv[0], "jobs") == 0)
    {
        // parsed_cmd->job_flag = true;
        // find_my_jobs(pm);
        print_my_jobs(pm);
        return 0;
    }
    // int status;
    // printf("wait %d\n%d\n", pm->PIDs[0], waitpid(pm->PIDs[0], &status, WNOHANG));
    int fds[2];
    if (pipe(fds) == -1)
    {
        return -1;
    }
    int ret = 0;
    pid_t pid = fork();
    if (pid == -1)
    {
        ret = -1;
    }
    else if (pid == 0)
    {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);
        ret =  call_Redi(parsed_cmd, total, index, outFDS);
        exit(ret);
    }
    else
    {
        // fprintf(stderr, "index is %d\n", index);
        // fflush(stderr);
        P_T_wait[index] = pid;
        // fprintf(stderr, "index is %d\n", index);
        // fflush(stderr);
        close(fds[1]);
        dup2(fds[0], 0);
        close(fds[0]);
        if (index < total - 1)
        {
            ret = exeWithPipe(parsed_cmd + 1, total, ++index, outFDS, pm, P_T_wait);
            // FREE_ALL(parsed_cmd);
        }
    }
    return ret;
}

bool find_bk(char* cmd, int index)
{
    index--;
    while (isspace(cmd[index]))
    {
        index--;
    }
    if (cmd[index] == '&')
    {
        cmd[index] = 0;
        return true;
    }
    else
    {
        cmd[++index] = 0;
        return false;
    }

}

bool is_missing(struct parse *parsed_cmd, int partition_number)
{
    for (int i = 0; i < partition_number; i++)
    {
        if (parsed_cmd[i].cmd == NULL || allspace(parsed_cmd + i) == 1)
        {
            printf("error: missing program\n");
            fflush(stdout);
            return true;
            break;
        }
    }// test missing program
    return false;
}

void error_print(ERROR_TYPE ERR, char *msg)
{
    switch (ERR)
    {
    case SYNTAX_ERROR:
    {
        my_cout(msg, "syntax error near unexpected token `", "'");
        break;
    }
    case DUPLI_IN:
    {
        my_cout("error: duplicated input redirection", "", "");
        break;
    }
    case DUPLI_OUT:
    {
        my_cout("error: duplicated output redirection", "", "");
        break;
    }
    default:
        break;
    }
    return;
}

int open_dir(char *direct)
{
    // int fd = open(direct, O_RDONLY);
    // if (fd == -1)
    // {
    //     printf("mumsh: cd: %s: No such file or directory\n", direct);
    // }
    // else
    // {

        // fprintf(stderr, " inside\n");
    int fch = chdir(direct);

        // fprintf(stderr, " fd is % d\n", fd);
    if (fch == -1)
    {
        fprintf(stderr, "%s: %s\n", direct, strerror(errno));
        return errno;
    } 
    return 0;
    // }
}

int cd_dir(char *direct)
{
    if (direct == NULL)
    {
        // direct = (char*)malloc(1);
        // direct[0] = '~';
        return -1;
    }
    // fprintf(stderr, "in cd dir : %s\n", direct);
    int name_len = (int)strlen(direct);
    // fprintf(stderr, " dir len: %d\n", name_len);
    int ret;
    if (direct[0] == '~' || name_len == 0)
    {
        // fprintf(stderr, " inside\n");
        struct passwd *usrname;
        usrname = getpwuid(getuid());
        int home_len = (int)strlen(usrname->pw_dir);
        char *New_dir = malloc(sizeof(char) * (size_t)(name_len + home_len));
        // fprintf(stderr, " dir len: %d\n", name_len + home_len);
        int i = 0;
        for (; i < home_len; i++)
        {
            New_dir[i] = usrname->pw_dir[i];
        }
        for (; i < home_len + name_len - 1; i++)
        {
            New_dir[i] = direct[i - home_len + 1];
        }
        New_dir[i] = 0;
        ret = open_dir(New_dir);
        free(New_dir);
    }
    else
    {
        // fprintf(stderr, " inside\n");
        // my_pwd();
        ret = open_dir(direct);
        // free(direct);
        // fprintf(stderr, "over\n");
        // my_pwd();
    }

        // fprintf(stderr, " over\n");
    // my_pwd();
    return ret;
}

void my_pwd()
{
    // fprintf(stderr, "my\n");

    char *route = getcwd(buf, MAX_COMMAND_LENGTH);
    if (route == NULL)
    {
        printf("Route not found\n");
        fflush(stdout);
    }
    else
    {
        printf("%s\n", buf);
        // fprintf("%s\n", buf);
        fflush(stdout);
    }
}

void wait_all()
{
    int ret2 = 0;
    while (1)
    {
        ret2 = wait(NULL);
        if (ret2 == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            break;
        }
    }
}

void prepare_quit(const int inFds, const int outFds, struct PID_manager *PID_m)
{
    for (int k = 0; k < PID_m->childP_number; k++)
    {
        free(PID_m->cmd_origin[k]);
    }
    dup2(inFds, STDIN_FILENO);
    dup2(outFds, STDOUT_FILENO);
    wait_all();
    printf("exit\n");
    fflush(stdout);
}

bool Read_cmd(char *wtchr, bool *ctn, char *cmd, int *index)
{
    int tmp;
    read_mode r_mode = IDLE;
    bool WT = false;
    int cnt_out = 0;
    while (1)
    {
        // printf("into inner %d  c %d\n", WT, cnt_out);
        fflush(stdin);
        tmp = getchar();
        if (WT == 1)
        {
            if ((char)tmp == '\n')
            {
                printf("> ");
                fflush(stdout);
                continue;
            }
            if (*wtchr == (char)tmp && *wtchr == '<')
            {
                if (*wtchr == (char)tmp)
                {
                    WT = false;
                    *ctn = true;
                }
            }
            if (*wtchr == (char)tmp && *wtchr == '>')
            {
                if (*wtchr == (char)tmp)
                {
                    if (cnt_out == 0 && cmd[(*index) - 1] == '>') 
                    {
                        cnt_out = 1;
                    }
                    else
                    {
                        WT = false;
                        *ctn = true;
                    }
                }
            }
            if (!isspace((char)tmp))
            {
                WT = false;
            }
        }

        /* If I press Ctrl + D, QUIT*/
        if (tmp == EOF)
        {
            return true;
        }

        switch (r_mode)
        {
        case IDLE:
        {
            if (tmp == '\'')
            {
                r_mode = SINGLE_LEFT;
            }
            else if (tmp == '\"')
            {
                r_mode = DOUBLE_LEFT;
            }
            else
            {
                if (tmp == '>' || tmp == '<' || tmp == '|')
                {
                    if (*ctn == 0)
                    {
                        WT = true;
                        *wtchr = (char)tmp;
                    }
                    cnt_out = 0;
                }
            }
            break;
        }
        case SINGLE_LEFT:
        {
            if (tmp == '\'')
            {
                r_mode = IDLE;
            }
            if ((char)tmp == '\n')
            {
                printf("> ");
                fflush(stdout);
            }
            break;
        }
        case DOUBLE_LEFT:
        {
            if (tmp == '\"')
            {
                r_mode = IDLE;
            }
            if ((char)tmp == '\n')
            {
                printf("> ");
                fflush(stdout);
            }
            break;
        }                
        default:
            break;
        }
        if ((char)tmp == '\n' && r_mode == IDLE)
        {
            return 0;
        }
        cmd[*index] =(char) tmp;
        *index += 1;
    }
}

int parse_exe_all(const int outFds, const int inFds, char* cmd, struct PID_manager *PID_m)
{
    struct parse parsed_cmd[512];
    /* pre cut cmd by pipeline*/
    int partition_number = pre_cut(cmd, parsed_cmd);
    // fprintf(stderr, "len is %ld", (unsigned long)partition_number);
    //     fflush(stderr);

    /* Find error missing program*/
    if (is_missing(parsed_cmd, partition_number) == 1)
    {
        return 0;
    }
    
    /* To store my own ps*/
    pid_t *P_T_wait = (pid_t*)malloc((size_t)partition_number * sizeof(pid_t));

    /* exe function*/
    int Exit_flag = exeWithPipe(&parsed_cmd[0], partition_number, 0, outFds, PID_m, P_T_wait);

    /* Free the malloced space*/
    for (int i = 0; i < partition_number; i++)
    {
        FREE_ALL(parsed_cmd + i);
    }

    
    /* check quit_flag for exit comman*/
    if (Exit_flag == EXIT)
    { 
        free(P_T_wait);
        prepare_quit(inFds, outFds, PID_m);
        return 1;
    }

    wait_mine(P_T_wait, partition_number);
    free(P_T_wait);
    /* wait for own comman*/
    dup2(inFds, STDIN_FILENO);
    dup2(outFds, STDOUT_FILENO);
    return 0;
}

void wait_mine(pid_t *PW, int total)
{
    int status;
    for (int i = 0; i < total; i++)
    {
        waitpid(PW[i], &status, 0);
    }
    return;
}
