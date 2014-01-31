#include "utils.h"


//func declarations
void execute_batch(char ** args);


int main(int argc, char** argv){
    //set the SIGINT HANDLER
    bind_signal(SIGINT, &parent_handler);
    bind_signal(SIGTERM, &child_handler);

    //initialising the two global variables
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
        IFBUG printf("MAIN LOOP : input is %s\n", input);  ENDBUG
		args = tokenize(input);	
		// Uncomment to print tokens
	    IFBUG printArgs(args);  ENDBUG

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
            childpid = fork();
            if(childpid > 0){ // parent
                IFBUG printf("parent : %d #run: childpid = %d\n ",getpid(), childpid);  ENDBUG
                int status;
                int p = waitpid(childpid, &status, 0);
                //childpid = -1;
                IFBUG printf("parent : %d #run: jash : child with pid %d returned\n",getpid(),childpid); ENDBUG
            }
            else{ // child
                IFBUG printf("child : %d #run : args - ", getpid()); ENDBUG
                IFBUG printArgs(args); ENDBUG
                execute_batch(args);
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
        else if(strcmp(args[0], "parallel") == 0){
            parallel(args);
        }
        else if(find_pipe_symbol(args) > 0){
            childpid = fork();
            if(childpid > 0){ // parent
                IFBUG printf("parent : %d #run: childpid = %d\n ",getpid(), childpid);  ENDBUG
                int status;
                int p = waitpid(childpid, &status, 0);
                //childpid = -1;
                IFBUG printf("parent : %d #run: jash : child with pid %d returned\n",getpid(),childpid); ENDBUG
            }
            else{ // child
                IFBUG printf("child : %d #run : args - ", getpid()); ENDBUG
                IFBUG printArgs(args); ENDBUG
                execute_pipe(args);
                //###  execl(x,x, "dummy1",NULL); //needs path to full executable
                exit(0);
            }
        }
        else{
            execute_command(args);
            IFBUG printf("returned from execute_command to MAIN\n");  ENDBUG
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
    IFBUG printf("execute_batch file name %s\n", myargs[1]); ENDBUG
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
			IFBUG printf("run EOF found\n"); ENDBUG
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
    }
    fclose(stream);
}


int findseperator(char ** args, int i){
    while(args[i]!=NULL){
        if(strcmp(args[i], ":::") == 0) return i;
        i++;
    }
    return i;
}

int parallel(char ** args){
    int cont = 1; //flag whether to continue scanning for commands in token array
    int i = 1; //last seen args index
    int tok = 1;//index where command starts
    IFBUG printArgs(args + 2);  ENDBUG
    while(cont){
        i = findseperator(args, i);
        if(args[i] == NULL){
            cont = 0;
        }
        args[i] = NULL; //now req arglist is [tok,i]
        IFBUG printf("parallel %d : command starts at %d, null at %d\n",getpid(), tok, i); ENDBUG
        /*  */
        execute_command_parallel(args+tok);
        /*  */
        i++;
        tok = i;
    }
    //waiting for all child processes to terminate
    wait();
    while(1){
        int status;
        int pid = wait(&status);
        if(pid == -1){ //error
            if(errno = ECHILD){ //calling process doesnot have any unwaited children
                IFBUG printf("parallel %d : No more child waiting can now exit\n", getpid()); ENDBUG
                break;
            }
        }
        else{
            IFBUG printf("parallel %d : child with pid %d collected\n", getpid(), pid); ENDBUG
        }
    }
}
