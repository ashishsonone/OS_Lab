#include "utils.h"

int main(){
    char * x =(char *) malloc(1000 * sizeof(char));
    strcpy(x, "ls -l | ls -l \n");
    int i;
    printf("%s" , x);
    char **y;
    y = tokenize(x);
    printArgs(y);
    printf("symbol at %d", find_pipe_symbol(y));
    i = find_pipe_symbol(y);
    y[i]= NULL;
    printf("%s\n", y[i+1]);
    printArgs(y+i+1);
    execvp(y[i+1], y+i+1);
}
