#include "signal.h"

void handler(int sig, siginfo_t *siginfo, void *context){
    printf("my id : %d , received signal %d\n", getpid(), sig);
}
void childaction(){
    int child_pid = fork();
    if(child_pid >0 ){ //parent
        printf("child (parent) : %d ##\n", getpid());
        int *status, res, option;
        status = malloc(sizeof(int));
        *status = 1;
        sleep(1);
        int p = waitpid(child_pid, status, 0);
        /* The calling thread suspends processing until status information is 
         * available for the specified child process, if the options argument is 0*/
        printf("child (parent) : %d waitpid %d\n",getpid(), p);
        if(p == child_pid){
            printf("child (parent) : %d status %d\n", getpid(),*status);
            if(WIFEXITED(*status)) printf("child (parent) : %d child exited properly\n", getpid());
        }
        printf("child (parent) : %d wait over\n", getpid());
    }
    else if(child_pid == 0){
        printf("grand child : %d ## \n", getpid());
        sleep(10);
        printf("grand child : %d sleep over\n", getpid());
    }

}

int main(){
    bind_signal(SIGINT, &handler);

    char * input = malloc(1000 * sizeof(char));
    FILE * stream = stdin;
    while(1){
        printf("$");
		char *in = fgets(input, 1000, stream); //taking input one line at a time
        //Checking for EOF
        if (in == NULL){
            printf("EOF found");  
            continue;
        }
        printf("%s\n", input);
        if(strcmp(input, "f\n") == 0){
            int child_pid = fork();
            if(child_pid >0 ){ //parent
                printf("parent : %d ##\n", getpid());
                int *status, res, option;
                status = malloc(sizeof(int));
                *status = 1;
                sleep(1);
                int p = waitpid(child_pid, status, 0);
                /* The calling thread suspends processing until status information is 
                 * available for the specified child process, if the options argument is 0*/
                printf("parent : %d waitpid %d\n",getpid(), p);
                if(p == child_pid){
                    printf("parent : %d status %d\n", getpid(),*status);
                    if(WIFEXITED(*status)) printf("parent : %d child exited properly\n", getpid());
                }
                printf("parent : %d wait over\n", getpid());
            }
            else if(child_pid == 0){
                printf("child : %d ##\n", getpid());
                //char *in = fgets(input, 1000, stream); //this forces the child to change state and hence parent's wait gets over
                //if(strcmp(input, "s") == 0) sleep(10);
                int i,j,k;
                printf("child : %d sleeping \n", getpid());
                childaction();
                printf("child : %d exiting \n", getpid());
                return 0;
            }
        }
    }
}
