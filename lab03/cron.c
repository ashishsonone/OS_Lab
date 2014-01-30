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
#define ALARMTIMEOUT 5

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
    char a;
} cron_task;

cron_task ** TaskList;  //List of cron tasks GLOBAL

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
        //cron_task *t = malloc(sizeof(cron_task));
       // t->command_tokens = malloc(MAXCRONTASK * sizeof(char**)); 
        TaskList[i] = malloc(sizeof(cron_task));
        TaskList[i]->command_tokens = malloc(MAXCRONTASK * sizeof(char**)); 
		char * in = fgets(input, MAXLINE, fp);				//we are storing the commands in an array
        //printf("line is %s" , input);
		if (in == NULL){
			if (DEBUG) printf("EOF found\n");				// terminate while loop upon ending of file input
            TaskList[i] = NULL;
			break;
		}
        char ** tokens = tokenize(input);
        printArgs(tokens);
        TaskList[i]->min = get_dot_value(tokens[0]);
        TaskList[i]->hour = get_dot_value(tokens[1]);
        TaskList[i]->mday = get_dot_value(tokens[2]);
        TaskList[i]->mon = get_dot_value(tokens[3]);
        TaskList[i]->wday = get_dot_value(tokens[4]);
        int kj =0;
        while(tokens[kj] !=NULL){
            printf("Here\n");
           TaskList[i]->command_tokens[kj] = malloc(sizeof(char) * 1000);
           strcpy(TaskList[i]->command_tokens[kj],tokens[kj]);

            kj++;
        }
        

        print_cron_task(TaskList[i]); 
        i++;
	}
    for(i=0; TaskList[i]!=NULL; i++){
        print_cron_task(TaskList[i]);
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
    printf("Command Called ");
    printArgs(list);
}

void  ALARMhandler(int sig)
{
    alarm(ALARMTIMEOUT);
    signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
    printf("\n\nalarm called\n");
    time_t * rawtime = malloc(sizeof(time_t));
    if(rawtime == NULL) {
        printf("malloc error");
        return;
    }
    struct tm * timeinfo;
    time (rawtime);
    timeinfo = localtime (rawtime); //Note pointer to tm struct
    
    printf("Curent Time : min %d, hour %d, mday %d, mon %d, wday %d\t", timeinfo->tm_min, timeinfo->tm_hour, timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_wday);

    int i;
    for(i=0; TaskList[i]!=NULL;i++){
        //print_cron_task(TaskList[i]);
        if(check_cron_time(TaskList[i], timeinfo) == 1){
            execute_command_parallel(TaskList[i]->command_tokens + 5);
        }
    }
    signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
}


int main(int argc, char** argv){
    /* register for alaram handler */
    signal(SIGALRM, ALARMhandler);
    /* END */
    printf("dot value : %d\n", get_dot_value("0"));
    TaskList = malloc(MAXCRONTASK * sizeof(cron_task*));
    int i=0;
    /*printf("printing tasklist\n");
    for(i=0; TaskList[i]!=NULL; i++){
        print_cron_task(TaskList[i]);
    }
      time_t rawtime;
      struct tm * timeinfo;

      time (&rawtime);
      timeinfo = localtime (&rawtime); //Note pointer to tm struct
      printf("Comparing with cron_task result %d" ,check_cron_time(TaskList[0], timeinfo)); 

      */
	//Allocating space to store the previous commands.
	int numCmds = 0;
	char **cmds = (char **)malloc(1000 * sizeof(char *));

	int printDollar = 1;

	char input[MAXLINE];
	char** tokens;

	int notEOF = 1;

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
    parse_cron_file(filename, TaskList);
    printf("printing tasklist\n");
    int i;
    for(i=0; TaskList[i]!=NULL; i++){
        print_cron_task(TaskList[i]);
    }
    alarm(2);
    while(1);
}
