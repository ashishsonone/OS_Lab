#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define MAXLINE 1000
#define DEBUG 0
#define IFBUG if(DEBUG==1){
#define ENDBUG }

typedef void (*sig_handler)(int, siginfo_t *, void *); //type for signal handler

//globals
int childpid;
int parentpid;


//function declarations 
void bind_signal(int signal, sig_handler handler);
void printArgs(char **args);
char ** tokenize(char* input);
int change_dir(char ** args);
int execute_command(char **args);
int execute_command_parallel(char **args);
void parent_handler(int sig, siginfo_t *siginfo, void *context);
void child_handler(int sig, siginfo_t *siginfo, void *context);
int find_pipe_symbol(char ** args);
void execute_pipe(char ** args);
int file_inout_parser(char** tokens);
int findseperator(char ** args, int i);
int parallel(char ** args);
void execute_batch(char ** myargs);
int run_jash_commands(char ** args);

//function definitions

void bind_signal(int signal, sig_handler handler){
    //printf ("this is the parent, with id %d\n", (int) getpid ());
    struct sigaction act;
 
    memset (&act, '\0', sizeof(act));
 
    /* Use the sa_sigaction field because the handles has two additional parameters */
    act.sa_sigaction = handler;
 
    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    act.sa_flags = SA_SIGINFO;
 
    if (sigaction(signal, &act, NULL) < 0) {
        perror ("sigaction");
    }
}

void printArgs(char **args){
    int i=0;
    printf("printing Args .. :");
    while((args[i] != NULL)){
        printf("\t%s ", args[i]);
        i++;
    }
    printf("\n");
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
        kill(parentpid, SIGINT);
    }
    return state;
}

int execute_command(char **args){
    childpid = fork(); //set the childpid
    if(childpid > 0){ // parent
        IFBUG printf("\nparent : %d #execute_command new child id=%d\n", getpid(), childpid); ENDBUG
        int status;
        int p = waitpid(childpid, &status, 0);
        IFBUG printf("parent : %d #execute_command waitpid returns %d\n",getpid(), p); ENDBUG
        if(p == childpid){
            IFBUG printf("parent : %d #execute_command waitpid status %d\n", getpid(),status); ENDBUG
            if(WIFEXITED(status)) {
                IFBUG printf("parent : %d #execute_command child exited properly\n", getpid()); ENDBUG
            }
        }
        IFBUG printf("parent : %d #execute_command wait over\n", getpid()); ENDBUG
        childpid = -1; //reset the childpid to -1
        //childpid = -1;
        return -1;
    }
    else{ // child
        IFBUG printf("child : %d #execute_command args : ", getpid()); ENDBUG
        IFBUG printArgs(args); ENDBUG
        execvp(args[0], args); 
        printf("error executing the command\n");
        IFBUG printf("child : %d #execute_command error executing the command\n", getpid()); ENDBUG
        //error, so send signal SIGINT to main program
        kill(parentpid, SIGINT);
        exit(0);
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
    }
}

int execute_command_parallel(char **args){
    childpid = fork(); //set the global childpid
    if(childpid > 0){ // parent
        IFBUG printf("\nparent : %d #execute_parallel new child id=%d\n", getpid(), childpid); ENDBUG
        IFBUG printf("parent : %d #execute_parallel wait over\n", getpid()); ENDBUG
        childpid = -1; //reset the childpid to -1
        //childpid = -1;
        return -1;
    }
    else{ // child
        IFBUG printf("child : %d #execute_parallel args : ", getpid()); ENDBUG
        IFBUG printArgs(args); ENDBUG
        execvp(args[0], args); 
        printf("error executing the command\n");
        IFBUG printf("child : %d #execute_parallel error executing the command\n", getpid()); ENDBUG
        //error, so send signal SIGINT to main program
        kill(parentpid, SIGINT);
        exit(0);
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
    }
}

void parent_handler(int sig, siginfo_t *siginfo, void *context)
{
    IFBUG printf("SIGINT getpid() %d parentpid %d childpid %d\n",getpid(), parentpid,childpid); ENDBUG

    if(getpid() == parentpid){
        if(childpid != -1 && childpid!=0){
            int child = childpid;
            childpid = -1;
            kill(child, SIGTERM);
            IFBUG printf("SIGINT parent %d : waiting for child to get killed\n", getpid()); ENDBUG
            int status;
            int p = waitpid(childpid, &status, 0);
            IFBUG printf("SIGINT parent %d : child %d got killd\n", getpid(), p); ENDBUG
       }
   }
}

void child_handler(int sig, siginfo_t *siginfo, void *context)
{
    IFBUG printf("SIGTERM : getpid() %d : Killing myself\n", getpid()); ENDBUG
    raise(SIGKILL);
}

/**** background symbol "&" checker  and sets it to NULL *******/
//return 1 on success(i.e if it is background) , 0 on failure(waitable)
find_if_background(char **args){
    int i;
    for(i=0;args[i] != NULL; i++){
        if(strcmp(args[i], "&") == 0){
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

/******* pipe command handling ******************/
int find_pipe_symbol(char ** args){
    int i;
    for(i=0;args[i] != NULL; i++){
        if(strcmp(args[i], "|") == 0){
            return i;
        }
    }
    return -1;
}

void execute_pipe(char ** args){
    int pos = find_pipe_symbol(args);
    args[pos] = NULL;
    int pipefd[2];

    /* get a pipe (buffer and fd pair) from the OS */
    if (pipe(pipefd)) {
        fprintf(stderr,"error creating pipe");
        exit(0);
    }

    int childpid = fork();
    if(childpid == -1) {
        fprintf(stderr,"error forking ");
        exit(0);
    }
    else if(childpid == 0){
        /* child */
        close(pipefd[0]); //close read end  
        dup2(pipefd[1], 1); //writeend => 1
        close(pipefd[1]);  
        /* exec ls */
        execvp(args[0], args);
        perror(args[0]);
        exit(0);
    }
    else{
        /* parent */
        /* do redirections and close the wrong side of the pipe */
        close(pipefd[1]); //close write end
        dup2(pipefd[0], 0); //readend => 0 
        close(pipefd[0]);  
        execvp(args[pos+1], args+ pos +1);
        perror(args[pos+1]);
        exit(0);
    }
}


/*** general run exectable command supporting < > >> etc ********/

int file_inout_parser(char** tokens){                   // this function checks for input and output files
    //char** tokens = tokenize(command);                    // the tokens are for the input commands
    int flag = 0;                                       // flag keeps track of file input/output operations, 
                                                        //  1 : input, 2 : output
    int newpid = 0;
    int input_found = 0;
    int output_found = 0;
    int inputfd;                                        // these are the fiel descriptors 
    int outputfd;
    int i = 0;                                          // i : index for iterating in the tokens 
//    printf("This is befor asdf sdf  foring \n");
    while (tokens[i]!=NULL){
        if (flag == 1){                                 // '<' found , if current token valid update input_found    
            input_found = i;                            //  
        flag = 0;
    }       
        else if (flag == 2){                            // ' > ' found , if current token valid outpu_found updated
            output_found = i;
            flag = 0;
    }
        else if (strcmp(tokens[i],"<") == 0){
            flag = 1;
            tokens[i] = NULL;
    }
        else if (strcmp(tokens[i],">") == 0){
            flag = 2;
            tokens[i] = NULL;
        }
        i++;
    }
//    printf("This is befor foring \n");
    newpid = fork();
    if (newpid == 0){
        if (flag != 0){                                     // ALERT!!!! check if input_found + output_found was invalid
            fprintf(stderr, "Invalid input/output file name, give file name was %s .\n", tokens[0]);
            return -1;
        }
        else if (input_found != 0 && output_found != 0){
            inputfd = open(tokens[input_found], O_RDONLY);                  // file descriptor : read only for input
            outputfd = open(tokens[output_found], O_WRONLY|O_CREAT,0640);                // file descriptor : write only for output
            dup2(inputfd,0);
            dup2(outputfd,1);
        }
        else if (input_found != 0){
            inputfd = open(tokens[input_found], O_RDONLY);                  // file descriptor : read only for input
            dup2(inputfd,0);
        }
        else if (output_found != 0){
            outputfd = open(tokens[output_found], O_WRONLY|O_CREAT,0640);                // file descriptor : write only for output
            dup2(outputfd,1);
        }
 //       printf("This is before execvp \n");

        execvp(tokens[0], tokens);

    }

    else {
        int status;
        waitpid(newpid, &status, 0);
    }
}


/****PARALLEL JASH COMMAND ********/
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

//execute the batch file given using "run" command
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
        
        run_jash_commands(args);
    }
    fclose(stream);
}

int run_jash_commands(char ** args){
    /* start interpreting the command tokens */
    int BACKFLAG = find_if_background(args);
    if(strcmp(args[0], "exit") == 0){
        printf("Bye!\n");
        killpg(getpgrp(), SIGTERM);
    }
    else if(strcmp(args[0], "cd") == 0){
        change_dir(args);
    }
    else if(strcmp(args[0], "run") == 0){
        childpid = fork();
        if(childpid > 0){ // parent
            int status;
            if(BACKFLAG == 0){
                int p = waitpid(childpid, &status, 0);
            }
            else{
                printf("Background pid : %d\n", childpid);
                childpid = -1;
            }
        }
        else{ // child
            signal(SIGINT, SIG_IGN);
            execute_batch(args);
            exit(0);
        }
    }
    else if(strcmp(args[0], "parallel") == 0){
        childpid = fork();
        if(childpid > 0){ // parent
            int status;
            if(BACKFLAG == 0){
                int p = waitpid(childpid, &status, 0);
            }
            else{
                printf("Background pid : %d\n", childpid);
                childpid = -1;
            }
        }
        else{ // child
            signal(SIGINT, SIG_IGN);
            parallel(args);
            exit(0);
        }
    }
    else if(find_pipe_symbol(args) > 0){
        childpid = fork();
        if(childpid > 0){ // parent
            int status;
            if(BACKFLAG == 0){
                int p = waitpid(childpid, &status, 0);
            }
            else{
                printf("Background pid : %d\n", childpid);
                childpid = -1;
            }
        }
        else{ // child
            signal(SIGINT, SIG_IGN);
            execute_pipe(args);
            exit(0);
        }
    }
    else{ //execute the executable with redirection symbols
        childpid = fork();
        if(childpid > 0){ // parent
            int status;
            if(BACKFLAG == 0){
                int p = waitpid(childpid, &status, 0);
            }
            else{
                printf("Background pid : %d\n", childpid);
                childpid = -1;
            }
        }
        else{ // child
            signal(SIGINT, SIG_IGN);
            file_inout_parser(args);
            exit(0);
        }
    }
}
