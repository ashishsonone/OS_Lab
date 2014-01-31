#include "utils.h"

int main(){
    char * x = malloc(1000 * sizeof(char));
    strcpy(x, "ls -l a a bc");
    //printf("%s" , x);
    char ** y = tokenize(x);
    int i;
    for(i=0;y[i]!=NULL; i++){
        printf("%s ", y[i]);
    }
    printf("%s \n", y[i]);
    return 0;
    //printArgs(y);
    printf("symbol at %d", find_pipe_symbol(y));
    y[i]= NULL;
    printf("%s\n", y[i+1]);
    printArgs(y+i+1);
    execvp(y[i+1], y+i+1);
}
