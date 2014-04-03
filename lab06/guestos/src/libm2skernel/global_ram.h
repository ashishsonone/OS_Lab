#ifndef GLOBAL_RAM_H
#define GLOBAL_RAM_H
#define RAM_FRAMES 1024*10

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ram_frame{
	unsigned char * data;
	int frame_id;
	int status; //if 1, then it is allocated to some process. process may/may not contain in this frame
}ram_frame;
 
typedef struct global_ram{
	ram_frame frames[RAM_FRAMES];
}global_ram;


void init_global_ram();

ram_frame get_free_ram_frame();
void release_ram_frame(int frame_id); //set flag to 0 in free_list

extern global_ram physical_ram;

#endif