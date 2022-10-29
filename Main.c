#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>
#include "parser.h"
#include "execute.h"
#include "history.h"


int OnlyAgain(char* line,int len)
{
    return line[0]=='a' && line[1]=='g' && line[2]=='a' && line[3]=='i' && line[4]=='n' 
            && (len==8||len==9);
    
}

int main(int argc, char const *argv[])
{
    int status = 1;

    path = malloc(PATH_MAX * sizeof(char));
    realpath("resources",path);

    while (status)
    {
        char prompt[] = "shell$";
        printf("\e[1m\x1b[32m%s ", prompt);
        printf("\x1b[0m");


        char *line;
        size_t line_size = 0;
        size_t read = 0;

        read = getline(&line, &line_size, stdin); //lee la entrada y crea una lista de todas las palabras

        if (line[0] != ' ' && !OnlyAgain(line,read))
        {
            SaveLine(line);
        }
        
        Semicolon_Split(line,read);

    }

    return 0;
}

