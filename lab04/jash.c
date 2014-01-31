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
        //check if args[0] is not null
        if(args[0] == NULL) continue;
        run_jash_commands(args);

	}
  
  
	printf("Print and deallocate %s\n", args[0]);
	// Freeing the allocated memory	
	for(i=0;args[i]!=NULL;i++){
		free(args[i]);
	}
	free(args);
	return 0;
}



