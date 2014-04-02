#include "vmm.h"

void init_vmm(){
	printf("initialising VMM. \n");
	hole_list_start = NULL;
	last_allocated_swap = 0;
}

uint32_t allocate_page(){
	uint32_t addr;
	if(hole_list_start != NULL){
		addr = hole_list_start->disk_addr;
		hole_list_start = hole_list_start->next;
		return addr;
	}

	addr = last_allocated_swap;
	last_allocated_swap+= 1;
	//printf("returning swap page number %d\n", addr);
	return addr;
}

uint32_t deallocate_page(uint32_t swap_page_addr){
	hole * new_head = (hole *) malloc(sizeof(hole));
	new_head->disk_addr = swap_page_addr;
	new_head->next = hole_list_start;
	hole_list_start = new_head;
	return swap_page_addr;
}

void read_swap_page(char * buff, uint32_t swap_page_addr){
	uint32_t start_byte = (swap_page_addr<<MEM_LOGPAGESIZE);
	FILE *fp = fopen("Sim_disk", "rb");
    if(fp == NULL){
        printf("Couldn't open HardDisk read");
        exit(0);
    }
    fseek(fp, start_byte, SEEK_SET);
    fread(buff, MEM_PAGESIZE, 1, fp);
    fclose(fp);
}

void write_swap_page(char * buff, uint32_t swap_page_addr){
	uint32_t start_byte = (swap_page_addr<<MEM_LOGPAGESIZE);
	FILE *fp = fopen("Sim_disk", "rb+");
    if(fp == NULL){
        printf("Couldn't open HardDisk write");
        exit(0);
    }
    fseek(fp, start_byte, SEEK_SET);
    fwrite(buff, MEM_PAGESIZE, 1, fp);
    fclose(fp);
}

void handle_page_fault(struct mem_t *mem, struct mem_page_t* page){
	uint32_t old_tag = mem->bound_logical_page_tag; // tag is the logical page no. 

	printf("Page Fault new: %u , old: %u ::: ",page->tag, old_tag );

	//printf("tag inside handle_page_fault : %u \n", mem->bound_logical_page_tag);
	if(old_tag!=-1){
		struct mem_page_t * old_page = mem_page_get(mem,old_tag);
		// write this page back to swap if it has become dirty
		old_page->dirty_bit=1;// for testing
		if(old_page->dirty_bit==1){	
			printf("Page out :%u",old_page->tag);	
			uint32_t disk_page = old_page->swap_page_no;
			unsigned char* buff = old_page->data;
			write_swap_page(buff,disk_page);
		}
		mem->bound_logical_page_tag = -1;
		old_page->frame_id = -1;
	//	printf("reset frameid -1\n");
		old_page->data = NULL;
		old_page->dirty_bit=0;
	}

	//printf("settinging frameid -1\n");
	printf("  Page in: %u\n", page->tag);
	page->frame_id = mem->allocated.frame_id;
	page->data = mem->allocated.data;
	page->dirty_bit=0;
	uint32_t disk_page = page->swap_page_no;
	read_swap_page(page->data, disk_page);
	page->valid_bit=1;	

	mem->bound_logical_page_tag = page->tag; 
	//printf("Swapping in done with new tag %u!\n", mem->bound_logical_page_tag);

}
/*
int main(){
	init_vmm();
	uint32_t p1 = allocate_page();
	printf("%d\n", p1);
	uint32_t p2 = allocate_page();
	printf("%d\n", p2);
	uint32_t p3 = allocate_page();
	printf("%d\n", p3);
	uint32_t x = deallocate_page(p3);
	p3 = allocate_page();
	printf("%d\n", p3);

	char * read_buff = malloc(MEM_PAGESIZE);	
	char * write_buff = malloc(MEM_PAGESIZE);	
	strcpy(write_buff, "sim disk write");

	write_swap_page(write_buff, p2);
	read_swap_page(read_buff, p2);

	printf("String is %s", read_buff);
}
*/