#include <stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */




int parse_args_from_string(char * line, char * args[], int * background_ptr){


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


char * get_string_stdin(char * line, size_t * buf_size_ptr){
    
    size_t line_size  =  getline(&line, buf_size_ptr, stdin);
    line[line_size - 1] = '\0';
    return line;

}


int prev_line_ptr(int line_ptr){
    return abs(1 - line_ptr); 
}


char * copy(char * str){
    size_t size = sizeof(str)/sizeof(char);
    char * temp = (char *)malloc(size * sizeof(char));

    memcpy(temp, str, sizeof(str));
    return temp;
}


int execute(char * args[], int background)
{
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
    return 0;
}
int main(void)
{

char* args[MAX_LINE/2 + 1], prev_args[MAX_LINE/2 + 1]; /* command line arguments */
char * lines[3] = {(char *) malloc(0), (char *) malloc(0), NULL} ;
const int LINE_BUFFER_INDEX = 2;
size_t buf_sizes[2] = {0, 0};
int line_ptr = 0;


int should_run = 1; /* flag to determine when to exit program */
int read_stdin = 1;
int spawn_child  = 1;

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        // run child in background?
        int background = 0;
 
        //command arg parsing
        
        if(read_stdin) lines[line_ptr] = get_string_stdin(lines[line_ptr], &buf_sizes[line_ptr]);
        else {
            read_stdin = 1;
            printf("%s\n", lines[line_ptr]);
        }

        lines[LINE_BUFFER_INDEX] =  copy(lines[line_ptr]);
        
        int argcount = parse_args_from_string(lines[LINE_BUFFER_INDEX], args, &background);


        if(argcount > 0){
            if(strcmp(args[0], "exit") == 0){
                should_run = 0;
            }else if(strcmp(args[0], "!!") == 0){
                if(buf_sizes[prev_line_ptr(line_ptr)] != 0){
                    read_stdin = 0;
                    line_ptr = prev_line_ptr(line_ptr);
                }else{
                    printf("%s", "No command found in history\n");
                }
            }else{
                int retcode  = execute(args, background);
                if(retcode == -1) return -1;
                line_ptr = prev_line_ptr(line_ptr);
            }
        }

        free(lines[LINE_BUFFER_INDEX]);
    }
    return 0;
}