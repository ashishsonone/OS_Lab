#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>      /* for 'ENOENT' and 'ENOMEM' */
#define MAXPARAMS 20
#define BUFFERSIZE 200


void parseArgs(char *input, char **args){
    //printf("first token is %s\n", "x");
    char *tok = strtok(input, " \n");
    int i=0;
    while(tok!=NULL){
        args[i++] = tok;
        tok = strtok(NULL, " \n");
    }
    args[i] = NULL;
}

void printArgs(char **args){
    int i=0;
    printf("printing args .. :\n");
    while((args[i] != NULL) && i < MAXPARAMS){
        printf("\t%s\n", args[i]);
        i++;
    }
}

int main(){

    char ** args = malloc(MAXPARAMS * (sizeof(char *)));
    char a[20] = "hello";
    args[0] = a;
    printf("line %s\n", args[0]);

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
            //printArgs(args);
            int pid = fork();
            if(pid > 0){ // parent
                //printf("parent: new child id =%d\n",pid);
                //printf("parent: waiting for child to execute command\n",pid);
                int pid = wait();
                //printf("parent: child with pid %d returned\n",pid);
            }
            else{ // child
                //printf("arg[0] %s\n", args[0]);
                //printArgs(args);
                execvp(args[0], args); 
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
    }
    /*
    int p1 = fork();
    if(p1 > 0){
        printf("parent: new child id =%d\n",p1);
    }
    else{
        char * args[] = {x,"dummy_VP",  NULL};
        execvp(x, args ); 
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
        return 0;
    }
    */
}
