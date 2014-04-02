#include <stdio.h>
#include <stdlib.h>
// #include "m2skernel.h"

/* Memory */

#define MEM_LOGPAGESIZE    12
#define MEM_PAGESHIFT      MEM_LOGPAGESIZE
#define MEM_PAGESIZE       (1<<MEM_LOGPAGESIZE)
#define MEM_PAGEMASK       (~(MEM_PAGESIZE-1))
#define MEM_PAGE_COUNT     1024

typedef struct hole{
	int disk_addr;
	struct hole * next;
} hole;

hole * hole_list_start; //TODO init this
int last_allocated_swap;

void init_vmm();

int allocate_page(); //return disk addr(of a free swap page). Helper function

int deallocate_page(int swap_page_addr);

void read_swap_page(char * buf, int swap_page_addr);
void write_swap_page(char * buf, int swap_page_addr);