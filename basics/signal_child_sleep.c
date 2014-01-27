#include "signal.h"

void handler(int sig, siginfo_t *siginfo, void *context){
    printf("my id : %d , received signal %d\n", getpid(), sig);
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
            printf("parent : %d ##", getpid());
            int child_pid = fork();
            if(child_pid >0 ){ //parent
                printf("parent : %d ##", getpid());
                int *status, res, option;
                status = malloc(sizeof(int));
                *status = 1;
                sleep(1);
                while(1){
                    //int p = waitpid(child_pid, status, WNOHANG);
                    /* if WNOHANG was specified and one or more child(ren) speci‐
                     * fied by pid exist, but have not yet changed state, then 0 is  returned.
                     * On error, -1 is returned.*/

                    int p = waitpid(child_pid, status, 0);
                    /* The calling thread suspends processing until status information is 
                     * available for the specified child process, if the options argument is 0*/
                    printf("waitpid %d\n", p);
                    if(p == child_pid){
                        printf("parent : %d status %d\n", getpid(),*status);
                        if(WIFEXITED(*status)) break;
                    }
                }
                printf("parent : %d wait over\n", getpid());
            }
            else if(child_pid == 0){
                printf("child : %d ##", getpid());
                //char *in = fgets(input, 1000, stream); //this forces the child to change state and hence parent's wait gets over
                //if(strcmp(input, "s") == 0) sleep(10);
                int i,j,k;
                printf("child : %d sleeping \n", getpid());
                sleep(10);
                printf("child : %d exiting \n", getpid());
                return 0;
            }
        }
    }
}
