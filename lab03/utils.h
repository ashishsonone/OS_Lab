#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#define MAXCRONTASK 100
#define ALARMTIMEOUT 5


#define MAXLINE 1000
#define DEBUG 0
#define IFBUG if(DEBUG==1){
#define ENDBUG }

typedef void (*sig_handler)(int, siginfo_t *, void *); //type for signal handler

typedef struct{
    int min;
    int hour;
    int mday;
    int mon;
    int wday;
} cron_task;


//globals
int childpid;
int parentpid;
char ** Commands;


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
    while((args[i] != NULL) && i < MAXLINE){
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
                    tokens[tokenNo] = (char*)malloc(1000 * sizeof(char));
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
				tokens[tokenNo] = (char*)malloc(1000 * sizeof(char));
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
    IFBUG printf("SIGINT getpid() %d parentpid %d childpid %d\n",getpid(), parentpid,childpid);  ENDBUG

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

int get_dot_value(char * t){ //get value in int of the five points denoting cron schedule
    // on * returns -1
    if(strcmp("*", t) == 0) return -1;
    return atoi(t);
}

void parse_cron_file(char * filename){
	FILE* fp = fopen(filename,"r");
    if(fp == NULL) printf("Error opening file");
    int i=0;
	while (!feof(fp)){
        char * input = malloc(MAXLINE* sizeof(char));
		char * in = fgets(input, MAXLINE, fp);				//we are storing the commands in an array
        if(in == NULL) {break;}
        Commands[i] = input;
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
    
    //printf("Curent Time : min %d, hour %d, mday %d, mon %d, wday %d\t", timeinfo->tm_min, timeinfo->tm_hour, timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_wday);
    int i;
    for(i=0; Commands[i] != NULL;i++){
        char ** t = tokenize(Commands[i]);
        cron_task c;
        c.min = get_dot_value(t[0]);
        c.hour = get_dot_value(t[1]);
        c.mday = get_dot_value(t[2]);
        c.mon = get_dot_value(t[3]);
        c.wday = get_dot_value(t[4]);
        if(check_cron_time(&c, timeinfo)){
            execute_command_parallel(t+5);
        }
        int i;
        char ** x = malloc(sizeof(char*));
        //free(x);
    }

    free(rawtime);
    signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
}

void cronfunction(char * filename){
    signal(SIGALRM, ALARMhandler);
    Commands = malloc(MAXCRONTASK * sizeof(char *));
    parse_cron_file(filename);
   /* int i;
    printf("printing commands \n");
    for(i=0; Commands[i] != NULL; i++){
        printf("%s", Commands[i]);
    }*/
    alarm(ALARMTIMEOUT);
    while(1){
    }
}
