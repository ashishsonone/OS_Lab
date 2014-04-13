#include "tlb.h"

void tlb_init(){
	int i;
	for(i=0;i<TLB_SIZE;i++) {
		tlb_entry[i].proc_id = -1;
		tlb_entry[i].tag = -1;
		tlb_entry[i].page = NULL;
	}
	next_tlb_pos = 0;
}

struct mem_page_t* tlb_lookup(uint32_t tag, int proc_id){
	//printf ("#status: looking up tlb entry for tag %u of process %d\n", tag, proc_id);
	
	int i;
	for(i=0;i<TLB_SIZE;i++)
		if (tag==tlb_entry[i].tag && proc_id==tlb_entry[i].proc_id) {
			printf ("tlb_lookup : #status: found tlb entry for tag %u of process %d\n", tag, proc_id);
			return tlb_entry[i].page;
		}
	
	printf ("tlb_lookup : #status: not found tlb entry for tag %u of process %d\n", tag, proc_id);

	return NULL;
}

void add_tlb_entry(struct mem_page_t* page, int proc_id){
	//printf ("#status: replacing tlb entry at index %d for process %d\n", next_tlb_pos, proc_id);

	tlb_entry[next_tlb_pos].proc_id = proc_id;
	tlb_entry[next_tlb_pos].page = page;
	tlb_entry[next_tlb_pos].tag = page->tag;
	
	next_tlb_pos = (next_tlb_pos+1)%TLB_SIZE;
}
