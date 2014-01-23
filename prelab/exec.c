#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>      /* for 'ENOENT' and 'ENOMEM' */

int main(){
    /* execl and execlp replaces the caller process */
    //execlp("echo","echo","hello !!",NULL);
    char x[] = "/home/ashish/Dropbox/ACADS/labs_sem6/os/lab03/prelab/run.sh";
    char xt[] = "/home/ashish/Dropbox/ACADS/labs_sem6/os/lab03/prelab/runt.sh";
    char x1[] = "./run.sh";

    // --------------------------PASSING ARGS AS ARRAY of char*-----------------------------
    int p1 = fork();
    if(p1 > 0){
        printf("parent: new child id =%d\n",p1);
    }
    else{
        char * args[] = {x,"dummy_VP",  NULL};
        execvp(x, args ); 
        //###  execl(x,x, "dummy1",NULL); //needs path to full executable
        return 0;
    }
    // -------------------------- PASSING ENV ---------------------------------
    int p2 = fork();
    if(p2 > 0){
        printf("parent: new child id =%d\n",p2);
    }
    else{
        char * env[] = {"DIR=dummy_LE_ENV", NULL};
        //execlp(x1,x1, "dummy2",NULL); //works with ./run.sh
        execle(xt,xt, "dummy_LE",NULL,env);
        if (errno == ENOENT)
            printf("child.exe not found in current directory\n");
        return 0;
    }
    // ----------------------------PASSING ARGS AS LIST arg0, arg1,.. -------------------------------
    int p3 = fork();
    if(p3 > 0){
        printf("parent: new child id =%d\n",p3);
    }
    else{
        execl(x,x, "dummy_L",NULL);
        return 0;
    }
    sleep(1);
    printf("exiting main process\n");
    return 0;
}
