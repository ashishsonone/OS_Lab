#ifndef GLOBAL_RAM_H
#define GLOBAL_RAM_H

#define RAM_FRAMES 1024*1024

typedef struct global_ram{
	int free_list[RAM_FRAMES];//containing 0 or 1. init to 0(free).  1(allocated)

	unsigned char * frames[RAM_FRAMES]; //these are initialised(malloced) at the start
}global_ram;


void init_global_ram();

unsigned char * get_free_ram_frame();
void release_ram_frame(int frame_id); //set flag to 0 in free_list

extern global_ram physical_ram;

#endif