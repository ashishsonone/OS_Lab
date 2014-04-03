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

struct allocated_frame* get_free_allocated_frame(struct mem_t* mem ){
	struct allocated_frame* head = mem->allocated_frames_head;
	while(head!=NULL){
		if(head->logical_page==NULL){
			printf("No need to page out.");
			return head;
		}
	}
	return NULL;
}

void page_out(struct mem_t *mem, struct mem_page_t* old_page){
	if(old_page==NULL){
		printf("Error : old page cannot be NULL in page out\n"	);
		exit(0);
	}
	old_page->dirty_bit=1;// for testing
	printf("setting dirty bit = 1 for testing\n");
	if(old_page->dirty_bit==1){	
		printf("Page out :%u",old_page->tag);	
		uint32_t disk_page = old_page->swap_page_no;
		unsigned char* buff = old_page->data;
		write_swap_page(buff,disk_page);
	}
	old_page->frame_id = -1;
	old_page->data = NULL;
	old_page->dirty_bit=0;
}



void handle_page_fault(struct mem_t *mem, struct mem_page_t* page){

	printf("Handling page fault..\n");
	allocated_frame* free_frame = get_free_allocated_frame(mem);
	if(free_frame==NULL){
		free_frame = dequeue_frame(mem); // dequeue will remove from queue itself
		page_out(mem, free_frame->logical_page);
		free_frame->logical_page = NULL;
	}

	if(isa_ctx == NULL)
		printf("isa_ctx NULL ; Page Fault new: %u , old: %u ; ", page->tag,	free_frame->logical_page->tag ); 
	else 
		printf("pid : %d, uid : %d ; Page Fault new: %u , old:	%u ; ",isa_ctx->pid, isa_ctx->uid, page->tag, free_frame->logical_page->tag );


	printf("  Page in: %u\n", page->tag);
	page->frame_id = (free_frame->frame).frame_id;
	page->data = (free_frame->frame).data;
	page->dirty_bit=0;
	uint32_t disk_page = page->swap_page_no;
	read_swap_page(page->data, disk_page);
	page->valid_bit=1;	
	free_frame->logical_page = page; 
	enqueue_frame(mem,free_frame);

}

void push_to_allocated_frames(struct mem_t* mem){
	allocated_frame* node = (allocated_frame*) malloc(sizeof(struct allocated_frame));
	node->frame = get_free_ram_frame();
	node->logical_page = NULL;

	if(mem->allocated_frames_head==NULL){
		mem->allocated_frames_head = node;
		mem->allocated_frames_head->next = NULL;
	}
	else{
		node->next = mem->allocated_frames_head;
		mem->allocated_frames_head = node;
	}
}

void allocate_initial_frames(struct mem_t* mem){
	// initial_page_frame_count
	int i=0;
	for(i=0;i<initial_page_frame_count;i++){
		push_to_allocated_frames(mem);		
	}
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