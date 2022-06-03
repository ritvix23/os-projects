#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */






int parse_args(char * args[], int * background_ptr){


    char * line =  (char * ) malloc(0);
    size_t buf_size = 0;
    size_t line_size  =  getline(&line, &buf_size, stdin);
    line[line_size - 1] = '\0';

    char delim[] = " ";
    char * saveptr = NULL;
    char * token = strtok_r(line, " ", &saveptr);

    int i = 0;
    while(token != NULL){
        args[i++] = token;
        token = strtok_r(NULL, " ", &saveptr);
    }


    // // terminate  args with a NULL
    if(i>0 && !strcmp(args[i-1], "&")){
        *background_ptr = 1;
        args[i-1] = NULL;
        return i-1;
    }
    args[i] = NULL;
    return i;
}


int main(void)
{
char* args[MAX_LINE/2 + 1]; /* command line arguments */

int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        // run child in background?
        int background = 0;

        //command arg parsing
        int argcount = parse_args(args, & background);
        

        // exit feature
        if(strcmp(args[0], "exit") == 0){
            should_run = 0;
            continue;
        }

        //spawn a child to execute the given command
        if(argcount  > 0){
            // fork a  child process
            int pid  = fork();

            if(pid == 0){
                // child
                // execute the command
                execvp(args[0], args);
            }else if(pid > 0){
                //parent

                //wait for the child?
                if(background == 0) wait(NULL);

            }else{
                printf("Error in forking child");
                return -1;
            }
        }
        



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