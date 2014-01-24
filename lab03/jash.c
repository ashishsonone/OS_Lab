#include "utils.h"


//func declarations
void execute_batch(char ** args);


int main(int argc, char** argv){
    //set the SIGINT HANDLER
    parent();

    parentpid = getpid();
    childpid = -1; 


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
            IFBUG printf("EOF found");  ENDBUG
            continue;
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
            change_dir(args);
        }
        else if(strcmp(args[0], "run") == 0){
            printf("Runnig executabe\n");
            int pid = fork();
            childpid = pid;
            if(pid > 0){ // parent
                IFBUG printf("parent: new child id =%d\n",pid); ENDBUG
                IFBUG printf("parent: waiting for child to execute command\n",pid); ENDBUG
                int pid = wait();
                childpid = -1;
                IFBUG printf("parent: jash : child with pid %d returned\n",pid); ENDBUG
            }
            else{ // child
                IFBUG printf("arg[0] %s\n", args[0]); ENDBUG
                IFBUG printArgs(args); ENDBUG
                execute_batch(args);
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
        else{
            execute_command(args);
            IFBUG printf("returned from execute_command to MAIN\n");  ENDBUG
            fflush(stdout);
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



//execute the batch file
void execute_batch(char ** myargs){
    int i;
    printf("file name %s\n", myargs[1]);
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
            break;
		}
        //printf("command is %s\n", cmd);
        args = tokenize(cmd);
        if(args[0] == NULL) {
            continue;
        }
        if(strcmp(args[0], "exit") == 0){
            printf("Bye!\n");
            //free(args);
            return;
        }
        else if(strcmp(args[0], "cd") == 0){
            int s = change_dir(args);
        }
        else{
            execute_command(args);
        }
        continue;
        //free(args);
        //free malloc
    }
}
