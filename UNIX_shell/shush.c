#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LINE 80 /* The maximum length command */






int parse_args(char * args[]){


    char * line =  (char * ) malloc(0);
    size_t buf_size = 0;
    size_t line_size  =  getline(&line, &buf_size, stdin);
    line[line_size - 1] = '\0';

    char delim[] = " ";
    char * saveptr = NULL;
    char * token = strtok_r(line, delim, &saveptr);

    int i = 0;
    while(token != NULL){
        args[i++] = token;
        token = strtok_r(NULL, delim, &saveptr);
    }

    return i;
}


int main(void)
{
char* args[MAX_LINE/2 + 1]; /* command line arguments */

int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);
    
        
        int argcount = parse_args(args);


        // should_run = 0;
        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */
    }
    return 0;
}