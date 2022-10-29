#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "execute.h"

int* is_command;                     //arreglo donde se almacenara si la i-esima palabra es un comando
int command_count;                   //cantidad de comandos que tendra la lista de comandos

char** split_line(char*line, int read)
{
    int count_w = 1;//cantidad de palabras
    char * temp_w;//palabra actual que estamos construyendo
    char** list_ws;//lista de palabras
    
    if(line[0]=='#')
    {
        read=0;
        list_ws=malloc(sizeof(char*));
        return list_ws;
    }

    for (size_t i = 0; i < read; i++)         //recorrido para contar la cantidad de palabras
    {
        if (line[i] == ' ')
        {
            count_w++;
            while (line[i+1]==' ')
            {
                i++;
            }
        }
        else if(line[i]=='|')
        {
            if (i==0)
            {
                perror("No command to pipe");
                exit(2);
            }
            else if(line[i-1]==' ' && line[i+1]==' ')
            {
                count_w--;
            }
            else if(line[i-1]!=' ' && line[i+1]!=' ')
            {
                count_w++;
            }
            
        }
        else if(line[i] == '#')
        {
            if(line[i-1]==' ')
            {
                count_w--;
            }
            break;
        }
    }   


    list_ws = malloc((count_w + 1)*sizeof(char*));
    is_command = calloc(count_w+1,sizeof(int));


    command_count = 1;
    int pos = 0;
    is_command[0] = 1;
    for (size_t i = 0; i < read; i++)      //recorrido para contar la cantidad de comandos y sus ubicaciones
    {
        if (line[i] == ' ')
        {
            pos++;
            while (line[i+1]==' ')
            {
                i++;
            }
        }
        else if(line[i]=='|')
        {
            command_count++;
            if(line[i-1]!=' ')
            {
                pos++;
                is_command[pos]=1;
                if(line[i+1]==' ')
                {
                    pos--;
                }
            }
            else
            {
                is_command[pos]=1;
                if(line[i+1]==' ')
                {
                    pos--;
                }
            }
        }
        else if(line[i] == '#')
        {
            break;
        }
    }
    

    char *ch;
    ch=strtok(line,"| \n\t");
    list_ws[0] = ch;
    for (size_t i = 1; i < count_w; i++)     //recorrido para separar las palabras
    {
        ch = strtok(NULL,"| \n\t");
        list_ws[i] = ch;
    }

    return list_ws;
}

command_t** command_list(char*line, int read)
{
    
    char** list_ws=split_line(line,read);//lista de palabras con split() hecho
    
    command_t** command_list = malloc((command_count + 1)*sizeof(command_t*));//arreglo donde se almacenan todos los comandos
    
    if(list_ws[0]==NULL)
    {
        return command_list;
    }

    int command_arguments[command_count]; //arreglo donde se almacenara la cantidad de argumentos de cada comando
    command_arguments[0]=1;
    int pos = 1;                              //se reinicia la variable pos para realizar otro recorrido a la lista de palabras
    int current_count = 0;                //numero del comando actual
    command_t* current = malloc(sizeof(command_t));      //comando que estamos construyendo
    command_list[0]=current;
    while (list_ws[pos]!=NULL)            //recorrido donde se almacenara la cantidad de argumentos y se actualizaran 
    {                                     //las propiedades ld,gd,qqd y pipe_coming que tiene cada comando
        if(!(strcmp(list_ws[pos],"<")))
        {
            current->ld=list_ws[pos+1];
            pos+=2;
        }
        else if(!(strcmp(list_ws[pos],">")))
        {
            current->gd=list_ws[pos+1];
            pos+=2;
        }
        else if(!(strcmp(list_ws[pos],">>")))
        {
            current->qqd=list_ws[pos+1];
            pos+=2;
        
        
        }
        else if(is_command[pos]==1)
        {
            current->before_pipe = 1;
            current = malloc(sizeof(command_t));
            current->after_pipe=1;
            current_count++;
            command_list[current_count]=current;
            pos++;
            command_arguments[current_count]=1;         //se inicializan todos comandos con argumento 1 porque tienen al menos
        }                                              
        else
        {
            command_arguments[current_count]++;
            pos++;
        }
    }

    current=command_list[0];
    current_count=0;                             //reiniciamos la variable para almacenar en ella el numero del comando actual
    current->args=malloc((command_arguments[0] + 1)*sizeof(char *));
    current->args[0]=list_ws[0];
    pos = 1;
    int current_arg=1;                          //numero del argumento actual
    while(list_ws[pos]!=NULL)                   //recorrido donde cada comando adquiere sus argumentos
    {
        if(is_command[pos]==1)
        {
            
            current_count++;
            current=command_list[current_count];
            current->args = malloc((command_arguments[current_count] + 1)*sizeof(char *));
            current->args[0]=list_ws[pos];
            current_arg=1;
            pos++;
            
        }
        else if(!(strcmp(list_ws[pos],"<") && strcmp(list_ws[pos],">") && strcmp(list_ws[pos],">>")))
        {
            pos+=2;
        }
        else
        {
            current->args[current_arg]=list_ws[pos];
            current_arg++;
            pos++;
        }
    }
    
    return command_list;
}

int And_Or_Split(char* ch)
{
    int last2 = 0;
    for (size_t k = 0; k <= strlen(ch); k++)
    {
        if(k==strlen(ch))
        {
            char * temp = calloc(k-last2,sizeof(char));
            for (size_t l = last2; l < k; l++)
            {
                temp[l-last2]=ch[l];
            }
            command_t **list_c = command_list(temp, strlen(temp));
            execute(list_c);
            free(temp);
        }
        else if((ch[k]=='&' && ch[k+1]=='&' && ch[k-1]==' ' && ch[k+2]==' '))
        {
            char * temp = calloc(k+1-last2,sizeof(char));
            for (size_t l = last2; l < k; l++)
            {
                temp[l-last2]=ch[l];
            }
            command_t **list_c = command_list(temp, strlen(temp));
            int exit_status = execute(list_c);
            if(exit_status != 0)
            {
                return exit_status;
            }
            free(temp);
            last2 = k + 2;
        }
        else if ((ch[k]=='|' && ch[k+1]=='|' && ch[k-1]==' ' && ch[k+2]==' '))
        {
            char * temp = calloc(k+1-last2,sizeof(char));
            for (size_t l = last2; l < k; l++)
            {
                temp[l-last2]=ch[l];
            }
            command_t **list_c = command_list(temp, strlen(temp));
            int exit_status = execute(list_c);
            if(exit_status == 0)
            {
                return exit_status;
            }
            free(temp);
            last2 = k + 2;
        }
        
        
    }
    return 0;
    
}

int Semicolon_Split(char* line, int read)
{
    int last = 0;
    for (size_t i = 0; i < read; i++)
    {
        if(line[i]==';'|| line[i]=='\n')
        {
            char * ch=calloc((i+1-last),sizeof(char));
            for (size_t j = last; j < i; j++)
            {
                ch[j-last]=line[j];
            }
            And_Or_Split(ch);
            free(ch);
            last = i+1;          
        }
    }
}