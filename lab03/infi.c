
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char * argv[]){
    int i;
    for(i=0;i < 5;i++){
        printf("infi.c :%s\n", argv[1]);
        //fflush(stdout);
        sleep(2); 
    }
    printf("infi.c : pid %d returning\n", getpid());
}
