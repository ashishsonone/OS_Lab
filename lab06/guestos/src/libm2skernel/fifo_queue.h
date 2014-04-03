#include "m2skernel.h"

// To avoid name clash: {Unique} HEAD
#define uhead mem->fifo_queue_head

void enqueue_frame (struct mem_t*, struct allocated_frame*);
struct allocated_frame* dequeue_frame (struct mem_t*);
struct allocated_frame * deque_this_allocated_frame(struct mem_t* mem, uint32_t tag);
void print_queue (struct mem_t* mem);
