#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "utils.h"

#define MAXLINE 1000
#define MAXCRONTASK 100

//declarations
void cronfunction(char*);
int checktime(char*);
int execute_command_dummy(char**);

typedef struct{
    int min;
    int hour;
    int mday;
    int mon;
    int wday;
    char **command_tokens;
} cron_task;

void print_cron_task(cron_task * t){
    printf("min %d, hour %d, mday %d, mon %d, wday %d\t", t->min, t->hour, t->mday, t->mon, t->wday);
    printArgs(t->command_tokens);
}

int get_dot_value(char * t){ //get value in int of the five points denoting cron schedule
    // on * returns -1
    if(strcmp("*", t) == 0) return -1;
    return atoi(t);
}

void parse_cron_file(char * filename, cron_task ** TaskList){
    char * input = malloc(MAXLINE* sizeof(char));
	FILE* fp = fopen(filename,"r");
    int i=0;
	while (1){
        cron_task * t = malloc(sizeof(cron_task));
		char * in = fgets(input, MAXLINE, fp);				//we are storing the commands in an array
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");				// terminate while loop upon ending of file input
            TaskList[i] = NULL;
			break;
		}
        char ** tokens = tokenize(input);
        t->min = get_dot_value(tokens[0]);
        t->hour = get_dot_value(tokens[1]);
        t->mday = get_dot_value(tokens[2]);
        t->mon = get_dot_value(tokens[3]);
        t->wday = get_dot_value(tokens[4]);
        t->command_tokens = tokens+5;
        TaskList[i] = t;
        i++;
	}
    fclose(fp);
}

int check_cron_time(cron_task *task, struct tm * time){
    if(task->min == -1 || task->min == time->tm_min){
        if(task->hour == -1 || task->hour == time->tm_hour){
            if(task->mday == -1 || task->mday == time->tm_mday){
                if(task->mon == -1 || task->mon == time->tm_mon){
                    if(task->wday == -1 || task->wday == time->tm_wday){
                        return 1;
                    }
                    return -1;
                }
                return -1;
            }
            return -1;
        }
        return -1;
    }
    return -1;
}

int execute_command_dummy(char** list){
	char* p;
	strcpy(p, "lol");
	printf("%s: ", p);
    printArgs(list);
}

int main(int argc, char** argv){
    printf("dot value : %d\n", get_dot_value("0"));
    cron_task ** TaskList = malloc(MAXCRONTASK * sizeof(cron_task*));
    parse_cron_file("cron.txt", TaskList);
    int i=0;
    printf("printing tasklist\n");
    for(i=0; TaskList[i]!=NULL; i++){
        print_cron_task(TaskList[i]);
    }
      time_t rawtime;
      struct tm * timeinfo;

      time (&rawtime);
      timeinfo = localtime (&rawtime); //Note pointer to tm struct
      printf("Comparing with cron_task result %d" ,check_cron_time(TaskList[0], timeinfo)); 
    return 0;
}
/*
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
		else{
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


void cronfunction(char* filename){
	int pid;
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
					execute_command_dummy(tokens+5);
					printf("this is the end, \n");
					exit(0);
					

				}

			}
			i++;   /// the outer while loop with an i
		}		


	}
}
*/
