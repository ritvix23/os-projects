#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/mman.h>


#define MAX_LINE 80 /* The maximum length command */

// indexes for switches -
const int BACKGROUND = 0, REDIRECTION = 1, REDIRECTION_FLOW = 2;

// indexes for loop_flags
const int READ_STDIN = 0, SHOULD_RUN = 1;

//values for switches[REDIRECTION_FLOW] - 
const int FILE_TO_PROGRAM = 0, PROGRAM_TO_FILE = 1;

//indexes for extras - 
const int FILE_PTR = 1, ARGCOUNT = 0;




int parseArgsFromString(char * line, char * args[]){

    char delim[] = " ";
    char * saveptr = NULL;
    char * token = strtok_r(line, " ", &saveptr);

    int i = 0;
    while(token != NULL){
        args[i++] = token;
        token = strtok_r(NULL, " ", &saveptr);
    }

    args[i] = NULL;
    return i;
}


char * getStringStdin(char * line, size_t * buf_size_ptr){
    
    size_t line_size  =  getline(&line, buf_size_ptr, stdin);
    line[line_size - 1] = '\0';
    return line;

}


int getPrevLinePtr(int line_ptr){
    return abs(1 - line_ptr); 
}


char * copy(char * str, size_t buf_size){
    char * temp = (char *)malloc(buf_size);

    memcpy(temp, str, buf_size);
    return temp;
}





void argProcessing(char * args[], int switches[], void *extras[]){


    int argcount  = *((int *)extras[ARGCOUNT]);

    //switch to run child in background
    switches[BACKGROUND] = 0;
    if(strcmp(args[argcount - 1], "&") == 0){
        switches[BACKGROUND] = 1;
        args[argcount - 1] = NULL;
        argcount--;
    }

    switches[REDIRECTION] = 0;
    for(int i = 0; i<argcount; i++){
        if(strcmp(args[i], ">")==0 || strcmp(args[i], "<") == 0){
            switches[REDIRECTION] = 1;
            switches[REDIRECTION_FLOW] = (strcmp(args[i], ">")==0? PROGRAM_TO_FILE : FILE_TO_PROGRAM); // redirection flow is program to file or file to program
            args[i] = NULL;
            extras[FILE_PTR] = args[i+1];
            // argcount  = i;

            for(int j = i; j+2<=argcount; j++){
            args[j] = args[j+2];
            }
            argcount -= 2;

        }
        
    }
    

    *((int *)extras[ARGCOUNT]) = argcount;
    
}


int execute(char * args[], int switches[], void * extras[])
{
    int background = switches[BACKGROUND], redirection = switches[REDIRECTION], redirection_flow = switches[REDIRECTION_FLOW];
    int pid  = fork();

    if(pid == 0){
        // child
        // execute the command

        if(redirection){
            char * file_name = extras[FILE_PTR];

            // if the file name is left empty
            if( file_name == NULL){
                printf("%s", "Error : Syntax error\n");
                return -1;
            }

            if(redirection_flow == FILE_TO_PROGRAM){
                int fd = open(file_name, O_RDONLY);
                if(fd == -1) {
                    printf("Error while opening the file : %s", file_name);
                    return -1;
                }
                dup2(fd, STDIN_FILENO);
            }else{
                int fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC);
                if( fd == -1){
                    printf("Error while opening the file : %s", file_name);
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
            }
        }

        execvp(args[0], args);
    }else if(pid > 0){
        //parent

        //wait for the child to finish
        if(background == 0) wait(NULL);

    }else{
        printf("Error in forking child");
        return -1;
    }
    return 0;
}


int main(void)
{

char* args[MAX_LINE/2 + 1];/* command line arguments */
char * lines[3] = {(char *) malloc(0), (char *) malloc(0), NULL} ;
const int LINE_BUFFER_INDEX = 2;
size_t buf_sizes[2] = {0, 0};
int line_ptr = 0;

void *extras[2] = {NULL, NULL}; 



int * argcntptr  = (int *)malloc(sizeof(int));
extras[ARGCOUNT] = argcntptr;

int switches[3] = {0, 0, 0};
int loop_flags[2] = {0, 0};

loop_flags[READ_STDIN] = 1;
loop_flags[SHOULD_RUN] = 1;

    while (loop_flags[SHOULD_RUN]) {
        


        printf("shush> ");
        fflush(stdout);

 
        //command arg parsing
        
        if(loop_flags[READ_STDIN] == 1) lines[line_ptr] = getStringStdin(lines[line_ptr], &buf_sizes[line_ptr]);
        else {
            loop_flags[READ_STDIN] = 1;
            printf("%s\n", lines[line_ptr]);
        }

        lines[LINE_BUFFER_INDEX] =  copy(lines[line_ptr], buf_sizes[line_ptr]);
        
        *((int  *) extras[ARGCOUNT]) = parseArgsFromString(lines[LINE_BUFFER_INDEX], args);
        
        argProcessing(args, switches, extras);

        int argcount = *((int *)extras[ARGCOUNT]);

        if(argcount > 0){
            if(strcmp(args[0], "exit") == 0){
                loop_flags[SHOULD_RUN]= 0;
            }else if(strcmp(args[0], "!!") == 0){
                if(buf_sizes[getPrevLinePtr(line_ptr)] != 0){
                    loop_flags[READ_STDIN] = 0;
                    line_ptr = getPrevLinePtr(line_ptr);
                }else{
                    printf("%s", "No command found in history\n");
                }
            }else{
                int retcode  = execute(args, switches, extras);
                if(retcode == -1) return -1;
                line_ptr = getPrevLinePtr(line_ptr);
            }
        }

        free(lines[LINE_BUFFER_INDEX]);
    }
    return 0;
}