#include "vmm.h"

void init_vmm(){
	hole_list_start = NULL;
	last_allocated_swap = 0;
}

int allocate_page(){
	int addr;
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

int deallocate_page(int swap_page_addr){
	hole * new_head = (hole *) malloc(sizeof(hole));
	new_head->disk_addr = swap_page_addr;
	new_head->next = hole_list_start;
	hole_list_start = new_head;
	return swap_page_addr;
}

void read_swap_page(char * buff, int swap_page_addr){
	int start_byte = (swap_page_addr<<MEM_LOGPAGESIZE);
	FILE *fp = fopen("Sim_disk", "rb");
    if(fp == NULL){
        printf("Couldn't open HardDisk read");
        exit(0);
    }
    fseek(fp, start_byte, SEEK_SET);
    fread(buff, MEM_PAGESIZE, 1, fp);
    fclose(fp);
}

void write_swap_page(char * buff, int swap_page_addr){
	int start_byte = (swap_page_addr<<MEM_LOGPAGESIZE);
	FILE *fp = fopen("Sim_disk", "rb+");
    if(fp == NULL){
        printf("Couldn't open HardDisk write");
        exit(0);
    }
    fseek(fp, start_byte, SEEK_SET);
    fwrite(buff, MEM_PAGESIZE, 1, fp);
    fclose(fp);
}
/*
int main(){
	init_vmm();
	int p1 = allocate_page();
	printf("%d\n", p1);
	int p2 = allocate_page();
	printf("%d\n", p2);
	int p3 = allocate_page();
	printf("%d\n", p3);
	int x = deallocate_page(p3);
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