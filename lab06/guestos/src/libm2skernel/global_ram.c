#include "global_ram.h"
#include "m2skernel.h"
#include <stdlib.h>
#include <stdio.h>

global_ram physical_ram;

void init_global_ram(){
	int i;
	for(i=0; i<RAM_FRAMES; i++){
		physical_ram.frames[i] = calloc(1, MEM_PAGESIZE);
		physical_ram.free_list[i] = 0;
	}
}

unsigned char *get_free_ram_frame(){
	int i;
	for(i=0; i<RAM_FRAMES; i++){
		if(physical_ram.free_list[i] == 0){
			physical_ram.free_list[i] = 1;
			return physical_ram.frames[i];
		}
	}

	printf("Should Not Reach Here. Memory exaushted. No more free frames in RAM\n");
	exit(0);
}

void release_ram_frame(int frame_id){
	if(frame_id < 0 || frame_id >= RAM_FRAMES){
		printf("Frame id out of bounds\n");
		exit(0);
	}
	physical_ram.free_list[frame_id] = 0;
}