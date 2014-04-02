#include "global_ram.h"
#include "m2skernel.h"
#include <stdlib.h>
#include <stdio.h>

global_ram physical_ram;

void init_global_ram(){
	int i;
	printf("initialising global ram .......: \n");
	for(i=0; i<RAM_FRAMES; i++){
		//printf("%d, ", i);
		physical_ram.frames[i].frame_id = i;
		physical_ram.frames[i].data = calloc(1, MEM_PAGESIZE);
		physical_ram.frames[i].status = 0;	
	}
	printf("done init global ram : \n");
}

ram_frame get_free_ram_frame(){
	int i;
	for(i=0; i<RAM_FRAMES; i++){
		if(physical_ram.frames[i].status == 0){
			physical_ram.frames[i].status = 1;
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
	physical_ram.frames[frame_id].status = 0;
}