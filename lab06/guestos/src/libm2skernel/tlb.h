#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "m2skernel.h"

#define TLB_SIZE 10

bool tlb_initialized;
int next_tlb_pos;

typedef struct tlb_entry_t {
	int proc_id;
	uint32_t tag;
	struct mem_page_t* page; 
};

struct tlb_entry_t tlb_entry[TLB_SIZE];

void tlb_init();
struct mem_page_t* tlb_lookup(uint32_t tag, int proc_id);
void add_tlb_entry(struct mem_page_t* page, int proc_id);
