struct page_node {
	struct mem_page_t *page;
	struct page_node *prev;
	struct page_node *next;
};

void insert_node(struct mem_t* mem, int addr) {
	struct page_node *node = (struct page_node*) calloc(1, sizeof(struct page_node));
	node->page = mem_page_get(mem,addr);
	node->prev = NULL;
	node->next = NULL;
	if(mem->head == NULL)
		mem->head = node;
	else {
		struct page_node* temp = mem->head;
		while(temp->next != NULL)
			temp = temp->next;
		temp->next = node;
		node->prev = temp;
	}
}

//not accounted for all_pinned yet.
//free list insertion
void delete_node(struct mem_t* mem){
	if(mem->head == NULL){
		//error msg?
		return;
	}

	else if(mem->head->next == NULL && mem->head->page->pinned == 0){
		//free list insertion, dirty pages, set page to not valid(assuming a valid int as 0 or 1) anymore
		mem->head->page->valid = 0;
		free(mem->head);
		mem->head = NULL;
	}
	else{
		struct page_node* temp = mem->head;
		int all_pinned = 1;
		while(temp != NULL){
			if(temp->page->pinned == 1)
				temp=temp->next;
			else if(temp->page->pinned == 0){
				all_pinned = 0;
				//free list insertion, dirty pages, set page to not valid(assuming a valid int as 0 or 1) anymore
				temp->page->valid = 0;
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
				free(temp);
				return;
			}
		}
		if(all_pinned == 1){
			//What to do in this case?
		}
	}
}