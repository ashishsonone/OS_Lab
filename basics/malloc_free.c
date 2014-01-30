/* free example */
#include <stdlib.h>     /* malloc, calloc, realloc, free */
#include <stdio.h>

#define CASE 3
#define NORM 0
#define UNIN 1
#define REFREE 2
#define MUSTDO 3

int main(){
    if(CASE == NORM){
        //normal
        char * c = malloc(100);
        printf("malloc pointer : %p\n",c);
        if(c == NULL) printf("Error Malloc\n");
        free(c);
    }

    if(CASE == UNIN){
        //free a uninitialised ptr  --> Error
        char *d;
        printf("uninitilalised ptr garbage : %p\n",d);
        if( d != NULL) printf("Uninitialised ptr not NULL\n");
        free(d);
    }

    if(CASE == REFREE){
        //free a already freed ptr  --> Error
        char * e = malloc(100);
        printf("malloc pointer : %p\n",e);
        if(e == NULL) printf("Error Malloc\n");
        free(e); //first time
        printf("even after free(ptr) malloc pointer ptr points to same address : %p\n",e);
        free(e);
    }

    if(CASE == MUSTDO){
        //How should you free a pointer so that refree doesn't cause error
        char * e = malloc(100);
        printf("malloc pointer : %p\n",e);
        if(e == NULL) printf("Error Malloc\n");
        free(e); //first time
        e = NULL; //immediately set ptr to NULL
        printf("Now ptr points to : %p\n",e);
        free(e);
    }
}
