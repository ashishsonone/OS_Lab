#include "utils.h"

int file_inout_parser(char** tokens){					// this function checks for input and output files
	//char** tokens = tokenize(command);					// the tokens are for the input commands
	int flag = 0;										// flag keeps track of file input/output operations, 
														//	1 : input, 2 : output
	int newpid = 0;
	int input_found = 0;
	int output_found = 0;
	int inputfd;										// these are the fiel descriptors 
	int outputfd;
	int i = 0;											// i : index for iterating in the tokens array
	while (tokens[i]!=NULL){
		if (flag == 1){									// '<' found , if current token valid update input_found 	
			input_found = i;							// 	
		flag = 0;
	}		
		else if (flag == 2){ 						    // ' > ' found , if current token valid outpu_found updated
			output_found = i;
			flag = 0;
	}
		else if (strcmp(tokens[i],'<') == 0){
			flag = 1;
			tokens[i] = NULL;
	}
		else if (strcmp(tokens[i],'>') == 0){
			flag = 2;
			tokens[i] = NULL;
		}
	}

	newpid = fork();
	if (newpid == 0){
		if (flag != 0){										// ALERT!!!! check if input_found + output_found was invalid
			fprintf(stderr, "Invalid input/output file name, give file name was %s .\n", tokens[0]);
			return -1;
		}
		else if (input_found != 0 && output_found != 0){
			inputfd = open(tokens[input_found], O_RDONLY);					// file descriptor : read only for input
			outputfd = open(tokens[output_found], O_WRONLY);				// file descriptor : write only for output
			dup2(inputfd,0);
			dup2(outputfd,1);
		}
		else if (input_found != 0){
			inputfd = open(tokens[input_found], O_RDONLY);					// file descriptor : read only for input
			dup2(inputfd,0);
		}
		else if (output_found != 0){
			outputfd = open(tokens[output_found], O_WRONLY);				// file descriptor : write only for output
			dup2(outputfd,1);
		}

		execvp(tokens[0], tokens);

	}

	else {
		int status;
		waitpid(newpid, &status, 0);
	}
}
