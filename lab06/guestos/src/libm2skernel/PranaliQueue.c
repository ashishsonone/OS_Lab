
struct queue_node
{
	struct allocated_frame * page_eqv;
	struct queue_node * prev, * next;
};

// To avoid name clash: {Unique} HEAD
#define uhead mem->fifo_queue_head

void enqueue (struct mem_t* mem, allocated_frame* next)
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
}

allocated_frame* dequeue (struct mem_t* mem)
{
	struct queue_node * curr = uhead;

	// Check if any node (effectively, page) exists in queue.
	while (curr != NULL)
	{
		// Check if (corresponding) page is pinned.
		if (curr->page_eqv->logical_page->pinned)
		{
			curr = curr->next;
			continue;
		}

		// Change the links in the queue.
		if (curr->prev != NULL) curr->prev->next = curr->next;
		if (curr->next != NULL) curr->next->prev = curr->prev;

		// Return first unpinned page equivalent.
		allocated_frame * res = curr->page_eqv;

		// Free up memory.
		free (curr);

		// Fix head if only node in queue.
		if (uhead == curr) uhead = NULL;

		return res;
	}

	return NULL;
}
