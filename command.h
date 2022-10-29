#ifndef command_h
#define command_h

typedef struct command
{
    char** args;
    char** flags;

    int before_pipe;
    int after_pipe;
    char* gd;
    char* ld;
    char * qqd;
    
}command_t;

typedef command_t * commandptr;

#endif 

