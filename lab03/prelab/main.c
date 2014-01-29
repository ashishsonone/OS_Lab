#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>      /* for 'ENOENT' and 'ENOMEM' */
#define MAXPARAMS 20
#define BUFFERSIZE 200
#define DEBUG 0
#define IFBUG if(DEBUG==1){
#define ENDBUG }


void parseArgs(char *input, char **args){
    IFBUG printf("first token is %s\n", "x"); ENDBUG
    char *tok = strtok(input, " \n");
    int i=0;
    while(tok!=NULL){
        args[i++] = tok;
        tok = strtok(NULL, " \n");
    }
    args[i] = NULL;
}


int main(){

    char ** args = malloc(MAXPARAMS * (sizeof(char *)));
    char a[20] = "hello";
    args[0] = a;
    IFBUG printf("line %s\n", args[0]);  ENDBUG

    char command[BUFFERSIZE] = "ls -l help";

    parseArgs(command, args);
    printArgs(args);

    while(1){
        printf(" $ ");
        fgets(command, BUFFERSIZE , stdin);
        parseArgs(command, args);
        if(args[0] == NULL) continue;
        if(strcmp(args[0], "exit") == 0){
            printf("Bye!\n");
            return 0;
        }
        else if(strcmp(args[0], "cd") == 0){
            chdir(args[1]);
        }
        else{
            IFBUG printArgs(args); ENDBUG
            int pid = fork();
            if(pid > 0){ // parent
                IFBUG printf("parent: new child id =%d\n",pid); ENDBUG
                IFBUG printf("parent: waiting for child to execute command\n",pid); ENDBUG
                int pid = wait();
                IFBUG printf("parent: child with pid %d returned\n",pid); ENDBUG
            }
            else{ // child
                IFBUG printf("arg[0] %s\n", args[0]); ENDBUG
                IFBUG printArgs(args); ENDBUG
                execvp(args[0], args); 
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
    }
}
