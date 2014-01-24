#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>


#define MAXLINE 1000
#define DEBUG 0

char* getday(char*);
char* getmonth(char*);

//declarations
char ** tokenize(char*);
void cronfunction(char*);
int checktime(char*);
int execute_command(char**);

int main(int argc, char** argv){

	//Setting the signal interrupt to its default function. 
	signal(SIGINT, SIG_DFL);

	//Allocating space to store the previous commands.
	int numCmds = 0;
	char **cmds = (char **)malloc(1000 * sizeof(char *));

	int printDollar = 1;

	char input[MAXLINE];
	char** tokens;

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
		tokens = tokenize(input);
		printf ("this is out tokeinze correspondent, %s\n", tokens[0]);	
		// here we go to the cron function which takes filename as argument
		if (strcmp(tokens[0], "cron" ) == 0){
			printf ("we have infiltrated cron facility\n");
	 		cronfunction(tokens[1]);
	 	}
		else {
			for(i=0;tokens[i]!=NULL;i++){
				printf("%s\n", tokens[i]);
			}
		}   
	}
  
  
	printf("Print and deallocate %s\n", tokens[0]);
	// Freeing the allocated memory	
	for(i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
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

void cronfunction(char* filename){
	int parentpid;
	int pid;
	int* childpids = (int*) malloc(sizeof(int));
	int cmds;
	cmds = 0;
	char** commands = (char **)malloc(1000 * sizeof(char *)); ;
	char input[MAXLINE];
	char * in;
	printf ("come on let me in i am BURning out here \n");
	FILE* fp = fopen(filename,"r");
	while (1){
		in = fgets(input, MAXLINE, fp);				//we are storing the commands in an array
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");				// terminate while loop upon ending of file input
			break;
		}
		
		commands[cmds] = (char *)malloc(sizeof(input));
		strcpy(commands[cmds++], input); 								// copying commands in commands array, 

	}
	// we have stored all the commands in the commands character arrray
	printf ("come on let me in i am freezing put here \n");
	
	while(1){
		sleep(2);     // sleeping for a minute
		int i = 0;	// total number if commands;
		while (i < cmds){
			int state = checktime(commands[i]);				// for each command check whether its time to run it
			
				printf("hey there i am free, %d\n", state);
				if (state == 1){
				pid = fork();							// if command is to be run at present, create a child and run it
				if (pid == 0){
					// its a child process  
					// now run the commands and exit
					// execute_command(char* command, char ** arglist);
					char** tokens;     /// now we apply the toeknize function
					tokens = tokenize(commands[i]);
					tokens = tokens + 5;
					printf("hey there i am inside please help, \n");
					
					printf("lets get down to businesss, \n");
					//strcpy(arglist[k+1], "\0");
					execute_command(tokens+5);
					printf("this is the end, \n");
					exit(0);
					

				}

			}
			i++;   /// the outer while loop with an i
		}		


	}
}

char* gettime(){

	time_t currtime;
	struct tm * info;
	time ( &currtime );
  	info = localtime( &currtime );
  	return asctime(info);
}

int gethour(){
	time_t currtime;
	struct tm * info;
	time ( &currtime );
  	info = localtime( &currtime );
  	return info->tm_hour;
}

int getminute(){
	time_t currtime;
	struct tm * info;
	time ( &currtime );
  	info = localtime( &currtime );
  	return info->tm_min;
}


int checktime(char* command){
	char** tokens = (char**) malloc (sizeof(char*));
	tokens = tokenize(command);
	char* currtime = (char*) malloc(sizeof(char));
	currtime = gettime();
	char** timetoken = (char**) malloc (sizeof(char*));
	timetoken = tokenize(currtime);
	int day;
	int month;
	int date;
	int hour;
	int min;
	if(strcmp(tokens[0],getday(timetoken[0]))==0 || strcmp(tokens[0],"*")==0){
		day = 1;
	}
	if(strcmp(tokens[1],getmonth(timetoken[1]))==0 || strcmp(tokens[1],"*")==0) {
		month = 1;
	}
	if(strcmp(tokens[2],timetoken[2])==0 || strcmp(tokens[2],"*")==0){
		date = 1;
	}
	if(strcmp(tokens[3],"*")==0){
		hour = 1;
	}
	else if(atoi(tokens[3])== gethour()){
		hour = 1;
	}
	if(strcmp(tokens[4],"*")==0){
		min = 1;
	}
	else if (atoi(tokens[4])==getminute()){
		min = 1;
	}

	if (day*month*date*hour*min == 1){
		return 1;
	}
	else {
		return 0;
	}

}

int execute_command(char** list){
	char* p;
	strcpy(p, "lol");
	printf("alright boys its rolling %s\n", p);
}

char* getday(char* day){
	if (strcmp(day, "Sun")==0) return "0";
	else if (strcmp(day, "Mon")==0) return "1";
	else if (strcmp(day, "Tue")==0) return "2";
	else if (strcmp(day, "Wed")==0) return "3";
	else if (strcmp(day, "Thu")==0) return "4";
	else if (strcmp(day, "Fri")==0) return "5";
	else if (strcmp(day, "Sat")==0) return "6";
	else return "-1";
}

char* getmonth(char* month){
	if (strcmp(month, "Jan")==0) return "1";
	else if (strcmp(month, "Feb")==0) return "2";
	else if (strcmp(month, "Mar")==0) return "3";
	else if (strcmp(month, "Apr")==0) return "4";
	else if (strcmp(month, "May")==0) return "5";
	else if (strcmp(month, "Jun")==0) return "6";
	else if (strcmp(month, "Jul")==0) return "7";
	else if (strcmp(month, "Aug")==0) return "8";
	else if (strcmp(month, "Sep")==0) return "9";
	else if (strcmp(month, "Oct")==0) return "10";
	else if (strcmp(month, "Nov")==0) return "11";
	else if (strcmp(month, "Dec")==0) return "12";
}
