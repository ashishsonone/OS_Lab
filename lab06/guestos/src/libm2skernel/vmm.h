#include <stdio.h>
#include <stdlib.h>
#include "m2skernel.h"

/* Memory */
typedef struct hole{
	uint32_t disk_addr;
	struct hole * next;
} hole;

hole * hole_list_start; //TODO init this
uint32_t last_allocated_swap;

void init_vmm();

uint32_t allocate_page(); //return disk addr(of a free swap page). Helper function

uint32_t deallocate_page(uint32_t swap_page_addr); // frees the disk swap page

void read_swap_page(char * buf, uint32_t swap_page_addr);
void write_swap_page(char * buf, uint32_t swap_page_addr);

void handle_page_fault(struct mem_t *mem, struct mem_page_t* page);
struct allocated_frame* get_free_allocated_frame(struct mem_t* mem );

void page_out(struct mem_t *mem, struct mem_page_t* old_page);
void allocate_initial_frames(struct mem_t* mem);