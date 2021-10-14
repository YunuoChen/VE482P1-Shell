#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "macro.h"
#include "exe_funct.h"


void parsed_fresh(struct parse *parsed_cmd)
{
    parsed_cmd->background_flag = false;
    parsed_cmd->cd_flag = false;
    // parsed_cmd->job_flag = false;
    parsed_cmd->cmd_num = 0;
    parsed_cmd->error_flag = NO_ERROR;
    parsed_cmd->error_msg[1] = '\0';
    parsed_cmd->is_dual = false;
    parsed_cmd->re_input = false;
    parsed_cmd->re_output = false;
    parsed_cmd->redirect_output = NULL;
    parsed_cmd->redirect_input = NULL;
    parsed_cmd->cd_direct = NULL;
    parsed_cmd->pwd_flag = false;
    for (int i = 0; i < MAX_COMMAND_LENGTH; i++)
    {
        parsed_cmd->argv[i] = NULL;
    }
}
void FREE_ALL(struct parse *parsed_cmd)
{
    // printf("in free\n");
    free(parsed_cmd->cd_direct);
    free(parsed_cmd->redirect_input);
    free(parsed_cmd->redirect_output);
    for (int i = 0; i < parsed_cmd->cmd_num; i++)
    {
        free(parsed_cmd->argv[i]);
        parsed_cmd->argv[i] = NULL;
    }
    parsed_cmd->redirect_output = NULL;
    parsed_cmd->redirect_input = NULL;
    parsed_cmd->cd_direct = NULL;
    // fprintf(stderr, "tes2\n\n\n\n");
}

int pre_cut(char *cmd, struct parse *parsed_cmd)
{
    int i = 0;
    int parse_index = 0;
    parsed_cmd[0].cmd = cmd;
    read_mode rm = IDLE;
    while (cmd[i] != 0)
    {
        switch (cmd[i])
        {
        case '\'':
        {
            if (rm == IDLE) rm = SINGLE_LEFT;
            else if (rm == SINGLE_LEFT) rm = IDLE;
            break;
        }
        case '\"':
        {
            if (rm == IDLE) rm = DOUBLE_LEFT;
            else if (rm == DOUBLE_LEFT) rm = IDLE;
            break;
        }
        case '|':
        {
            if (rm == IDLE)
            {
                cmd[i] = 0;
                parsed_cmd[++parse_index].cmd = cmd + i + 1;
            }
            break;   
        }
        default:
            break;
        }
        i++;
    }
    // printf("%d\n", parse_index + 1);
    return parse_index + 1;
}

void my_split(char* cmd, struct parse* parsed_cmd)
{
    // parsed_init(parsed_cmd);
    // parsed_fresh(parsed_cmd);
    // printf("begin cmd is %s\n%d\n", cmd, parsed_cmd->cmd_num);
    int len = (int) strlen(cmd);
    int i = 0;
    int arg_index = 0;
    parsed_fresh(parsed_cmd);
    read_mode R_mode = IDLE;
    // bool continue_read = false;
    // read_mode inARGV = IDLE;
    while (cmd[i] != 0 && parsed_cmd->error_flag == NO_ERROR && i < (int)strlen(cmd))
    {
        // printf("index %d\n%c\n", i, cmd[i]);
    // printf("begin cmd is %s\n%d\n%s\n", cmd, parsed_cmd->cmd_num, parsed_cmd->argv[0]);
        switch (R_mode)
        {
        case IDLE:
        {
            if (isspace(cmd[i]))
            {
                i++;
            }
            else if (cmd[i] == '<')
            {
                R_mode = REDIRECT_IN;
                if (i == len - 1)
                {
                    parsed_cmd->error_flag = SYNTAX_ERROR;
                    parsed_cmd->error_msg[0] = '|';
                }
                i++;
                break;
            }
            else if (cmd[i] == '>')
            {
                R_mode = REDIRECT_OUT;
                if (i == len - 1)
                {
                    parsed_cmd->error_flag = SYNTAX_ERROR;
                    parsed_cmd->error_msg[0] = '|';
                }
                i++;
                break;
            }
            else if (cmd[i] == 'c' && cmd[i + 1] == 'd' && cmd[i + 2] == ' ')
            {
                R_mode = CD_R;
                i += 3;
            }
            else if (cmd[i] == 'p' && cmd[i + 1] == 'w' && cmd[i + 2] == 'd' && (cmd[i + 3] == ' ' || cmd[i+3] == '\n' || cmd[i+3] == '\0'))
            {
                // fprintf(stderr, " her \n");
                // fflush(stderr);
                parsed_cmd->pwd_flag = true;
                i += 4;
            }
            else
            {
                R_mode = ARGV;
            }
            break;
        }

        case REDIRECT_IN:
        {
            if (parsed_cmd->re_input == 1)
            {
                parsed_cmd->error_flag = DUPLI_IN;
                parsed_cmd->error_msg[0] = '<';
                break;
            }
            else
            {
                parsed_cmd->re_input = true;
                while (isspace(cmd[i]))
                {
                    i++;
                }
                if (cmd[i] == '<' || cmd[i] == '>')
                {
                    parsed_cmd->error_flag = SYNTAX_ERROR;
                    parsed_cmd->error_msg[0] = cmd[i];
                    break;
                }
                else
                {
                    R_mode = IN_FILENAME;
                }
            }
            break;
        }

        case REDIRECT_OUT:
        {
            // fprintf(stderr,"first in out %c\n", cmd[i]);
            // fflush(stdout);
            // cmd[i - 1] = 0;
            if (parsed_cmd->re_output == 1)
            {
                // printf("error11\n");
                parsed_cmd->error_flag = DUPLI_OUT;
                parsed_cmd->error_msg[0] = '>';
                break;
            }
            else
            {
                parsed_cmd->re_output = true;
                if (cmd[i] == '>')
                {
                    parsed_cmd->is_dual = true;
                    i++;
                }
                while (isspace(cmd[i]))
                {
                    i++;
                }
                // fprintf(stderr, "i is %d, len is %d", i, len);
                if (i == len)
                {
                    parsed_cmd->error_flag = SYNTAX_ERROR;
                    parsed_cmd->error_msg[0] = '|';
                    break;
                }
                if (cmd[i] == '<' || cmd[i] == '>')
                {
                    // printf("error ape\n");
                    // fflush(stdout);
                    parsed_cmd->error_flag = SYNTAX_ERROR;
                    parsed_cmd->error_msg[0] = cmd[i];
                    break;
                }
                else
                {
                    R_mode = OUT_FILENAME;
                }
            }
            // printf("over out %c\n", cmd[i]);
            // fflush(stdout);
            break;
        }

        case IN_FILENAME:
        {
            // printf("begin cmd[%d] is %c\n", i, cmd[i]);
            //     fflush(stdout); 
            parsed_cmd->redirect_input = (char*)malloc(MAX_COMMAND_LENGTH);
            parsed_cmd->re_input = true;
            int index = 0;
            bool m_Single = false;
            bool m_Double = false;
            while ((!isspace(cmd[i]) || m_Single == 1 || m_Double == 1) && cmd[i] != '\0' )
            {
                // printf("cmd[%d] is %c\n", i, cmd[i]);
                // fflush(stdout);
                if (cmd[i] == '\'')
                {
                    if (m_Single == false && m_Double == false)
                    {
                        m_Single = true;
                        i++;
                        continue;
                    }
                    else if (m_Single == true)
                    {
                        m_Single = false;
                        i++;
                        continue;
                    }
                }
                if (cmd[i] == '\"')
                {
                    if (m_Double == false && m_Single == false)
                    {
                        m_Double = true;
                        i++;
                        continue;
                    }
                    else if (m_Double == true)
                    {
                        m_Double = false;
                        i++;
                        continue;
                    }
                }
                if (m_Double == false && m_Single == false)
                {
                    if (cmd[i] == '>')
                    {
                        // printf("to ifndcmd[%d] is %c\n", i, cmd[i]);
                        // printf("encounter i is %d, len is %d\n", i, len);
                        // fflush(stdout);
                        R_mode = REDIRECT_OUT;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        // i++;
                        break;
                        // continue_read = false;
                    }
                    else if (cmd[i] == '<')
                    {
                        // i++;
                        R_mode = REDIRECT_IN;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        break;
                    }

                }
                parsed_cmd->redirect_input[index++] = cmd[i++];
                // printf("cmd[%d] is %c\n", i, cmd[i]);
                // fflush(stdout);
            }
            parsed_cmd->redirect_input[index] = 0;
            // printf("finish %s\n", parsed_cmd->redirect_input);
            // cmd[i] = 0;
            // i++;
            if (R_mode == IN_FILENAME) R_mode = IDLE;
            break;
        }
        case OUT_FILENAME:
        {
            parsed_cmd->redirect_output = (char*)malloc(MAX_COMMAND_LENGTH);
            // printf("in read\n");
            // printf("cmd[%d] is %c\n", i, cmd[i]);
            // fflush(stdout);
            parsed_cmd->re_output = true;
            int index = 0;
            bool m_Single = false;
            bool m_Double = false;
            while ((!isspace(cmd[i]) || m_Single == 1 || m_Double == 1) && cmd[i] != '\0' )
            {
                // printf("cmd[%d] is %c\n", i, cmd[i]);
                // fflush(stdout);
                if (cmd[i] == '\'')
                {
                    if (m_Single == false && m_Double == false)
                    {
                        m_Single = true;
                        i++;
                        continue;
                    }
                    else if (m_Single == true)
                    {
                        m_Single = false;
                        i++;
                        continue;
                    }
                }
                if (cmd[i] == '\"')
                {
                    if (m_Double == false && m_Single == false)
                    {
                        m_Double = true;
                        i++;
                        continue;
                    }
                    else if (m_Double == true)
                    {
                        m_Double = false;
                        i++;
                        continue;
                    }
                }
                if (m_Double == false && m_Single == false)
                {
                    if (cmd[i] == '>')
                    {
                        R_mode = REDIRECT_OUT;
                        // i++;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        break;
                        // continue_read = false;
                    }
                    else if (cmd[i] == '<')
                    {
                        // i++;
                        R_mode = REDIRECT_IN;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        break;
                    }

                }
                parsed_cmd->redirect_output[index++] = cmd[i++];
                // printf("out cmd[%d] is %c\n", i, cmd[i]);
                // fflush(stdout);
            }
            parsed_cmd->redirect_output[index] = 0;
            // printf("finish %s\n", parsed_cmd->redirect_output);
            // cmd[i] = 0;
            i++;
            if (R_mode == OUT_FILENAME) R_mode = IDLE;
            break;
        }
        case ARGV:
        {
    // printf("begin cmd is %s\n%d\n%s\n", parsed_cmd->cmd, parsed_cmd->cmd_num, parsed_cmd->argv[0]);
            parsed_cmd->argv[arg_index] = (char*)malloc(MAX_COMMAND_LENGTH);
            int index = 0;
            read_mode fmd = IDLE;
            while (cmd[i] != '\0' && R_mode == ARGV && (!isspace(cmd[i]) || fmd == SINGLE_LEFT || fmd == DOUBLE_LEFT))
            {
                // printf("cmd[%d] is %c\n", i, cmd[i]);
                switch (fmd)
                {
                case IDLE:
                {
                    if (cmd[i] == '\'')
                    {
                        fmd = SINGLE_LEFT;
                    }
                    else if (cmd[i] == '\"')
                    {
                        fmd = DOUBLE_LEFT;
                    }
                    else if (cmd[i] == '<')
                    {
                        R_mode = REDIRECT_IN;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        break;
                    }
                    else if (cmd[i] == '>')
                    {
                        R_mode = REDIRECT_OUT;
                        if (i == len - 1)
                        {
                            parsed_cmd->error_flag = SYNTAX_ERROR;
                            parsed_cmd->error_msg[0] = '|';
                        }
                        i++;
                        break;
                    }
                    else if (!isspace(cmd[i]))
                    {
                        // printf("add %c\n", cmd[i]);
                        parsed_cmd->argv[arg_index][index++] = cmd[i];
                    }
                    i++;
                    break;
                }
                case SINGLE_LEFT:
                {
                    if (cmd[i] == '\'')
                    {
                        fmd = IDLE;
                    }
                    else
                    {
                        parsed_cmd->argv[arg_index][index++] = cmd[i];
                    }
                    i++;
                    break;
                }
                case DOUBLE_LEFT:
                {
                    if (cmd[i] == '\"')
                    {
                        fmd = IDLE;
                    }
                    else
                    {
                        parsed_cmd->argv[arg_index][index++] = cmd[i];
                    }
                    i++;
                    break;
                }
                default:
                    break;
                }
            }
            parsed_cmd->argv[arg_index++][index] = 0;
            // fprintf(stderr, "index is %d\n", index);
            // printf("%s\n%d\n", parsed_cmd->argv[0],parsed_cmd->argv[0][4] == 0);
            // printf("finish %s\n i is %d\n", parsed_cmd->argv[arg_index - 1], arg_index);
            // printf("begin cmd is %s\ni is %d", cmd, i);
            // cmd[i] = 0;
            // i++;
            if (R_mode == ARGV) R_mode = IDLE;
            break;
        }
        case CD_R:
        {
            parsed_cmd->cd_direct = (char*)malloc(MAX_COMMAND_LENGTH);
            parsed_cmd->cd_flag = true;
            while (isspace(cmd[i]))
            {
                i++;
            }
            // parsed_cmd->cd_direct = cmd + i;
            // printf("here%d\n%s", !isspace(cmd[i]), cmd + i);
            int index = 0;;
            bool m_Single = false;
            bool m_Double = false;
            while ((!isspace(cmd[i]) || m_Single == 1 || m_Double == 1) && cmd[i] != '\0' )
            {
                if (cmd[i] == '\'')
                {
                    if (m_Single == false && m_Double == false)
                    {
                        m_Single = true;
                        i++;
                        continue;
                    }
                    else if (m_Single == true)
                    {
                        m_Single = false;
                        i++;
                        continue;
                    }
                }
                if (cmd[i] == '\"')
                {
                    if (m_Double == false && m_Single == false)
                    {
                        m_Double = true;
                        i++;
                        continue;
                    }
                    else if (m_Double == true)
                    {
                        m_Double = false;
                        i++;
                        continue;
                    }
                }
                parsed_cmd->cd_direct[index++] = cmd[i++];
            }
            // fprintf(stderr, "index is %d\n", index);
            parsed_cmd->cd_direct[index] = '\0';
            // cmd[i] = 0;
            // i++;
            if(R_mode == CD_R) R_mode = IDLE;
            // printf("after P1 == is %d\n%c\n%c\n%ld\n", strcmp(parsed_cmd->cd_direct, "P1\0"), parsed_cmd->cd_direct[0], parsed_cmd->cd_direct[1], strlen(parsed_cmd->cd_direct));
            break;
        }
        default:
            break;
        }
    }
    // printf("parse over %d\n", parsed_cmd->redirect_output == NULL);
    // if (parsed_cmd->re_input == 1 && parsed_cmd->redirect_input == NULL)
    // {
    //     parsed_cmd->error_flag = SYNTAX_ERROR;
    //     parsed_cmd->error_msg[0] = '|';
    // }
    // if (parsed_cmd->re_output == 1 && parsed_cmd->redirect_output == NULL)
    // {
    //     // printf("insd\n");
    //     parsed_cmd->error_flag = SYNTAX_ERROR;
    //     parsed_cmd->error_msg[0] = '|';
    // }
    parsed_cmd->argv[arg_index] = NULL;
    // printf("after P1 == is %d\n%c\n%c\n%ld\n", strcmp(parsed_cmd->cd_direct, "P1\0"), parsed_cmd->cd_direct[0], parsed_cmd->cd_direct[1], strlen(parsed_cmd->cd_direct));
    parsed_cmd->cmd_num = arg_index;
}
