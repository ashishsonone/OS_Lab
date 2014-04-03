#include "fifo_queue.h"

void print_queue (struct mem_t* mem)
{
	struct queue_node * curr = uhead;
	while (curr != NULL)
	{
		printf ("%d", curr->page_eqv->logical_page);
		if ((curr = curr->next))
			printf (", ");
	}
	printf ("\n");
}

void enqueue_frame (struct mem_t* mem, struct allocated_frame* next)
{
	// Create the next node.
	struct queue_node* new_node = (struct queue_node*) calloc (1, sizeof (struct queue_node));
	// Set the next node to point to given page equivalent.
	new_node->page_eqv = next;
	new_node->next = NULL;

	if (uhead == NULL)
	{
		// Enqueue at the uhead of [empty] queue.
		new_node->prev = NULL;
		uhead = new_node;
	}
	else
	{
		// Reach the last node.
		struct queue_node* temp = uhead;
		while (temp->next != NULL)
			temp = temp->next;

		// Enqueue after the temp node.
		new_node->prev = temp;
		temp->next = new_node;
	}

	printf ("enqueue_frame : %d\n", next->logical_page); // print_queue (mem);
}

struct allocated_frame* dequeue_frame (struct mem_t* mem)
{
	struct queue_node * curr = uhead;

	// Check if any node (effectively, page) exists in queue.
	while (curr != NULL)
	{
		// Check if (corresponding) page is pinned.
		if (curr->page_eqv->logical_page->isPinned)
		{
			printf ("dequeue_frame : skipping pinned page\n"); // print_queue (mem);
			curr = curr->next;
			continue;
		}

		// Change the links in the queue.
		if (curr->prev != NULL) curr->prev->next = curr->next;
		if (curr->next != NULL) curr->next->prev = curr->prev;

		// Return first unpinned page equivalent.
		allocated_frame * res = curr->page_eqv;

		// Fix head if only node in queue.
		if (uhead == curr) uhead = curr->next;
		
		// Free up memory.
		free (curr);

		printf ("dequeue_frame : %d\n", res->logical_page); // print_queue (mem);
		return res;
	}

	printf ("dequeue_frame : no frame in queue\n"); // print_queue (mem);
	return NULL;
}
