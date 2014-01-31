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
#define ALARMTIMEOUT 10

//declarations
void cronfunction(char*);
int checktime(char*);


typedef struct{
    int min;
    int hour;
    int mday;
    int mon;
    int wday;
} cron_task;

char ** Commands;

int get_dot_value(char * t){ //get value in int of the five points denoting cron schedule
    // on * returns -1
    if(strcmp("*", t) == 0) return -1;
    return atoi(t);
}

void parse_cron_file(char * filename){
    printf("file name is%s\n", filename);
	FILE* fp = fopen(filename,"r");
    int i=0;
	while (!feof(fp)){
        char * input = malloc(MAXLINE* sizeof(char));
		char * in = fgets(input, MAXLINE, fp);				//we are storing the commands in an array
        if(in == NULL) {printf("null found\n");break;}
        Commands[i] = input;
        printf("command %d is %s", i, Commands[i]);
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
    
    printf("Curent Time : min %d, hour %d, mday %d, mon %d, wday %d\t", timeinfo->tm_min, timeinfo->tm_hour, timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_wday);
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
    parse_cron_file(filename);
    int i;
    printf("printing commands \n");
    for(i=0; Commands[i] != NULL; i++){
        printf("%s", Commands[i]);
    }
    alarm(2);
}

int main(int argc, char** argv){
    /* register for alaram handler */
    signal(SIGALRM, ALARMhandler);
    /* END */
    printf("dot value : %d\n", get_dot_value("0"));
    printf("size of crontask * %d\n", sizeof(cron_task*));
    Commands = malloc(MAXCRONTASK * sizeof(char *));
    printf("size of crontask * %d\n", sizeof(cron_task*));
    cronfunction("cron.txt");
    while(1){
    }
	return 0;
}
