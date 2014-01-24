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

//declarations
char ** tokenize(char*);
void changedir(char ** args);
void executebatch(char ** args);

int main(int argc, char** argv){

	//Setting the signal interrupt to its default function. 
	signal(SIGINT, SIG_DFL);

	//Allocating space to store the previous commands.
	int numCmds = 0;
	char **cmds = (char **)malloc(1000 * sizeof(char *));

	int printDollar = 1;

	char input[MAXLINE];
	char** args;

	int notEOF = 1;
	int i;

	FILE* stream = stdin;

	while(notEOF) { 
		if (printDollar == 1){ 
			printf("$ "); // the prompt
			fflush(stdin);
		}

		char *in = fgets(input, MAXLINE, stream); //taking input one line at a time

		//Checking for EOF
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");
			exit(0);
		}

		//add the command to the command list.
		cmds[numCmds] = (char *)malloc(sizeof(input));
		strcpy(cmds[numCmds++], input); 

		// Calling the tokenizer function on the input line    
		args = tokenize(input);	
		// Uncomment to print tokens
	 
		for(i=0;args[i]!=NULL;i++){
			printf("%s\n", args[i]);
		}

        /* start interpreting the command tokens */
        if(args[0] == NULL) continue;
        if(strcmp(args[0], "exit") == 0){
            printf("Bye!\n");
            return 0;
        }
        else if(strcmp(args[0], "cd") == 0){
            changedir(args);
        }
        else if(strcmp(args[0], "run") == 0){
            printf("Runnig executabe\n");
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
                executebatch(args);
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
        else{
            printf("Wrong command\n");
        }
	}
  
  
	printf("Print and deallocate %s\n", args[0]);
	// Freeing the allocated memory	
	for(i=0;args[i]!=NULL;i++){
		free(args[i]);
	}
	free(args);
	return 0;
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
void changedir(char ** args){
    printf("Changing directory\n");
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
}

//execute args
void executebatch(char ** myargs){
    printf("file name %s", myargs[1]);
	FILE* stream = fopen(myargs[1], "r");  //call a function to get the absolute path of file

    if(stream == NULL){
        printf("Batch file %s could not be opened\n", myargs[1]);
        return;
    }
	while(!feof(stream)) { 
		char *cmd = (char *)malloc(1000 * sizeof(char));
        char ** args;
		char *in = fgets(cmd, MAXLINE, stream); //taking input one line at a time

		//Checking for EOF
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");
            free(cmd);
            free(args);
            break;
		}
        printf("command is %s\n", cmd);
        args = tokenize(cmd);
        if(args[0] == NULL) {
            free(cmd);
            continue;
        }
        if(strcmp(args[0], "exit") == 0){
            printf("Bye!\n");
            free(cmd);
            free(args);
            return;
        }
        else if(strcmp(args[0], "cd") == 0){
            changedir(args);
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
        //free malloc
    }
}
