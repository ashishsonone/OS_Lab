#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char *str, *tok;
    str = malloc(23);
    strcpy(str, "hello,ashish+@Inspiron speaking");
    tok = strtok(str, ", +");
    printf("tok %s \n", tok);
    tok = strtok(NULL, ", +");
    printf("tok %s \n", tok);
    tok = strtok(NULL, ", +");
    printf("tok %s \n", tok);
    printf("rem %s \n-------------\n", str);
}
