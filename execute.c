#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include "execute.h"
#include "parser.h"
#include "history.h"

//comprueba si un comando es builtin
int is_builtin(command_t* command)
{
    return !(strcmp(command->args[0], "cd")&&strcmp(command->args[0],"help")
            &&strcmp(command->args[0],"exit")&&strcmp(command->args[0],"true")
            &&strcmp(command->args[0],"false")&&strcmp(command->args[0],"history")
            &&strcmp(command->args[0],"again"));
}

int exec_cd(char* new_path)
{
    int a = chdir(new_path);
    if (a!=0)
    {
        printf("cd: %s Not such file or directory\n",new_path);
    }
    return a;
}
int exec_help(command_t* command)
{
    char* help_arg = calloc(15,sizeof(char));
    strcat(help_arg, "/");
    if(command->args[1]!=NULL)
    {
        strcat(help_arg,command->args[1]);
    }
    else
    {
        help_arg = "/help";
    }

    char* full_path = calloc(PATH_MAX,sizeof(char));
    strcat(full_path,path);
    strcat(full_path,help_arg);
    strcat(full_path,".txt");
    char current[1000];
    FILE* file = fopen(full_path,"r");
    if(file==NULL)
    {
        printf("Not such file or directory\n");
        return 7;
    }
    while(1)
    {
        fgets(current,999,file);
        if(feof(file))
        {
            break;
        }
        printf("%s",current);
    }
    fclose(file);
    return 0;
}
int exec_exit()
{
    exit(0);
}
int exec_history(char * count_l)
{
    int exc = ReadLines(atoi(count_l));
    return exc;
}
int exec_again(char * hist_num)
{
    int n = atoi(hist_num);
    int exc = -1;
    if (0 < n && n < 11)
    {
        char* line = GetHistoryLine(10-n+1);
        SaveLine(line);
        exc = Semicolon_Split(line,(int)strlen(line));
    }
    else
    {
        printf("Argument Out of Range");
    }
    return exc;
}
int exec_true()
{
    return 0;
}
int exec_false()
{
    return 1;
}

int exec_builtin(command_t* command)
{
    char* name = command->args[0];

    int exc = 0;
    if (strcmp(name,"cd")==0)
    {
        exc = exec_cd(command->args[1]);
    }
    if (strcmp(name,"help")==0)
    {
        exc = exec_help(command);
    }
    if (strcmp(name,"exit")==0)
    {
        exc = exec_exit();
    }
    if(strcmp(name,"true")==0)
    {
        exc = exec_true();
    }
    if(strcmp(name,"false")==0)
    {
        exc = exec_false();
    }
    if(strcmp(name,"history")==0)
    {
        if (command->args[1]!=NULL)
        {
            exc = exec_history(command->args[1]);
        }
        else
        {
            exc = exec_history("10");
        }
    }
    if(strcmp(name,"again")==0)
    {
        exc = exec_again(command->args[1]);
    }
    return exc;
}

int execute(command_t** commands)
{
    int pos = 1;
    int pipes_count = 0;
    while (commands[pos]!=NULL)
    {
        pipes_count++;
        pos++;
    }
    
    int fd_old[2];
    int fd_new[2];

    int exstatus; 
    int i = 0;
    while (commands[i] != NULL)
    { 
        command_t* test = commands[i]; 
        if (is_builtin(commands[i]))
        {
            exstatus = exec_builtin(commands[i]);
        }
        else
        {
            if(i<pipes_count)
            {
                pipe(fd_new);
            }
            command_t* current = malloc(sizeof(command_t));
            current = commands[i];
            int pid = fork();
            if(pid == -1)
            {
                perror("Process Fork error");
                return 2;
            }
            if(pid==0)   //Proceso hijo
            {
                if(current->gd != NULL)
                {
                    int file = open("tester.txt", O_WRONLY | O_CREAT , 0777);
                    if(file == -1)  
                    {
                        printf("Writing File Error");
                        return 3;
                    }
                    dup2(file, STDOUT_FILENO);
                    close(file);
                }
                if (current->ld != NULL)
                {
                    int file = open("tester.txt", O_RDONLY, 0777);
                    if(file == -1)
                    {
                        printf("Reading File Error");
                        return 4;
                    }
                    dup2(file, STDIN_FILENO);
                    close(file);
                }
                if (current->qqd != NULL)
                {
                    int file = open("tester.txt", O_APPEND | O_WRONLY | O_CREAT , 0777);
                    if(file == -1)  
                    {
                        printf("Writing File Error");
                        return 5;
                    }
                    dup2(file, STDOUT_FILENO);
                    close(file);
                }
                if(current->after_pipe != 0)
                {
                    dup2(fd_old[0],STDIN_FILENO);
                    close(fd_old[0]);
                    close(fd_old[1]);
                }
                if(current->before_pipe != 0)
                {
                    close(fd_new[0]);
                    dup2(fd_new[1],STDOUT_FILENO);
                    close(fd_new[1]);
                }
                exstatus = execvp(current->args[0],current->args);
                if(exstatus == -1)
                {
                    printf("Could not find program to execute\n");
                    return 6;
                }
            }
            else        //Proceso padre
            {
                if(current->after_pipe != 0)
                {
                    close(fd_old[0]);
                    close(fd_old[1]);
                }
                if(current->before_pipe != 0)
                {
                    fd_old[0] = fd_new[0];
                    fd_old[1] = fd_new[1];
                }
            }
        }
        i++;
    }
    for (size_t j = 0; j < i; j++)
    {
        wait(NULL);
    }  

    return exstatus;
}
