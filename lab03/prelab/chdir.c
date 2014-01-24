#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>      /* for 'ENOENT' and 'ENOMEM' */

int main(){
    chdir("/home/ashish/Desktop/");
    char * args[] = {"ls", NULL};
    execvp(args[0], args ); 
}

