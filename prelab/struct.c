#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct point{
    int x;
    int y;
    char name[20];
};

typedef struct{
    int x;
    int y;
    char name[20];
} mypoint;

void print(struct point * p){
    printf("point %s is (%d, %d)\n", p->name, p->x, p->y);
}
void myprint(mypoint * mp){
    printf("mypoint %s is (%d, %d)\n", mp->name, mp->x, mp->y);
}

int main(){
    struct point p = {.x = 2, .y = 43, .name="origin"};
    print(&p);
    mypoint mp = {.x = 2, .y = 43, .name="origin"};
    myprint(&mp);
}
