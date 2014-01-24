#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>


#define MAXLINE 1000
#define DEBUG 0
#define IFBUG if(DEBUG==1){
#define ENDBUG }


//globals
int childpid;
int parentpid;

void printArgs(char **args){
    int i=0;
    printf("printing args .. :\n");
    while((args[i] != NULL) && i < MAXLINE){
        printf("\t%s\n", args[i]);
        i++;
    }
}
/*the tokenizer function takes a string of chars and forms tokens out of it*/
char ** tokenize(char* input){
	int i;
	int doubleQuotes = 0;
	
	char *token = (char *)malloc(1000*sizeof(char));
	int tokenIndex = 0;

	char **tokens;
	tokens = (char **) malloc(MAXLINE*sizeof(char**));
 
	int tokenNo = 0;
	
	for(i =0; i < strlen(input); i++){
		char readChar = input[i];
		
		if (readChar == '"'){
			doubleQuotes = (doubleQuotes + 1) % 2;
			if (doubleQuotes == 0){
				token[tokenIndex] = '\0';
				if (tokenIndex != 0){
					tokens[tokenNo] = (char*)malloc(sizeof(token));
					strcpy(tokens[tokenNo++], token);
					tokenIndex = 0; 
				}
			}
		}
		else if (doubleQuotes == 1){
			token[tokenIndex++] = readChar;
		}
		else if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(sizeof(token));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else{
			token[tokenIndex++] = readChar;
		}
	}
	
	if (doubleQuotes == 1){
		token[tokenIndex] = '\0';
		if (tokenIndex != 0){
			tokens[tokenNo] = (char*)malloc(sizeof(token));
			strcpy(tokens[tokenNo++], token);
		}
	}
	
	return tokens;
}

//change directory give the args
//  {"cd" , path , NULL }
int change_dir(char ** args){
    IFBUG printf("Changing directory\n");  ENDBUG
    int state;
    if(args[1] == NULL){
        state = chdir("/home/ashish");
    }
    else{
        state = chdir(args[1]);
    }
    if(state == -1){
        printf("Error changing directory\n");
    }
    return state;
}

int execute_command(char **args){
    IFBUG printArgs(args); ENDBUG
    int pid = fork();
    childpid = pid;
    IFBUG printf("setting childpid ACTUAL.. i am process : %d\n", getpid());  ENDBUG
    if(pid > 0){ // parent
        IFBUG printf("parent: new child id =%d\n",pid); ENDBUG
        IFBUG printf("parent: waiting for child to execute command\n",pid); ENDBUG
        waitpid(pid, 0, 0); //important for wait so that child doesnt become zombie
        //childpid = -1;
        IFBUG printf("parent: execute_commad : child with pid %d returned\n",pid); ENDBUG
        return -1;
    }
    else{ // child
        IFBUG printf("arg[0] %s\n", args[0]); ENDBUG
        IFBUG printArgs(args); ENDBUG
        execvp(args[0], args); 
        printf("error in executing the command\n");
        //error, so send signal SIGINT to main program
        kill(parentpid, SIGINT);
        exit(0);
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
    }
}

int execute_command_parallel(char **args){
    printf("-------------------------  ");
    IFBUG printArgs(args); ENDBUG
    int pid = fork();
    if(pid > 0){ // parent
        IFBUG printf("parent: new child id =%d\n",pid); ENDBUG
        IFBUG printf("parent: waiting for child to execute command\n",pid); ENDBUG
        IFBUG printf("parent: execute_commad : child with pid %d returned\n",pid); ENDBUG
        return -1;
    }
    else{ // child
        IFBUG printf("arg[0] %s\n", args[0]); ENDBUG
        IFBUG printArgs(args); ENDBUG
        execvp(args[0], args); 
        printf("error in executing the command\n");
        //error, so send signal SIGINT to main program
        kill(parentpid, SIGINT);
        exit(0);
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
    }
}

void parent_handler(int sig, siginfo_t *siginfo, void *context)
{
    IFBUG printf("parent %d child %d\n",parentpid,childpid);  ENDBUG

   if(getpid() == parentpid){
       if(childpid != -1 && childpid!=0){
            int child = childpid;
            printf("killing the child%d %d\n",parentpid,childpid);
            childpid = -1;
            kill(child, SIGTERM);
            printf("waiting for child to get killed\n");
            int x = waitpid(child, 0, 0);
            printf("child %d got killd\n", x);
       }
   }
}

void parent(){
    //printf ("this is the parent, with id %d\n", (int) getpid ());
    struct sigaction act;
 
    memset (&act, '\0', sizeof(act));
 
    /* Use the sa_sigaction field because the handles has two additional parameters */
    act.sa_sigaction = parent_handler;
 
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;
 
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror ("sigaction");
    }
}
void child_handler(int sig, siginfo_t *siginfo, void *context)
{
    printf("Killing myself\n");
    raise(SIGKILL);
}

void child(){
    //printf ("this is the parent, with id %d\n", (int) getpid ());
    struct sigaction act;
 
    memset (&act, '\0', sizeof(act));
 
    /* Use the sa_sigaction field because the handles has two additional parameters */
    act.sa_sigaction = child_handler;
 
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;
 
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        perror ("sigaction");
    }
}


void myfree(void *v){
    if(v!=NULL) free(v);
}
