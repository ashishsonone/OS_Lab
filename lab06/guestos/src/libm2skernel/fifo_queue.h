#include "m2skernel.h"

// To avoid name clash: {Unique} HEAD
#define uhead mem->fifo_queue_head

void enqueue_frame (struct mem_t*, struct allocated_frame*);
struct allocated_frame* dequeue_frame (struct mem_t*);
