/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <m2skernel.h>
#include <sys/mman.h>
#include "global_ram.h"

/* Total space allocated for memory pages */
unsigned long mem_mapped_space = 0;
unsigned long mem_max_mapped_space = 0;

/* Safe mode */
int mem_safe_mode = 1;
int *disk_protection_map;
int disk_protection_map_size; //=no of blocks

struct mem_page_t *page_table_lookup(struct mem_t* mem, uint32_t addr)
{
	uint32_t index, tag;
	struct mem_page_t *page;

	tag = addr & ~(MEM_PAGESIZE - 1);

	if (isa_ctx) {
		page = tlb_lookup (tag, isa_ctx->pid);
		if (page!=NULL) return page;
	}

	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];
	/* Look for page */
	while (page && page->tag != tag)
		page = page->next;

	if (isa_ctx)
		if (page!=NULL)
			add_tlb_entry(page, isa_ctx->pid);

	if(!page) printf("#error: page not found (in lookup by addr = %u)\n", addr);

	return page;
}

/* Return mem page corresponding to an address. */
struct mem_page_t *mem_page_get(struct mem_t *mem, uint32_t addr)
{	
	//printf("mem_page_get called for addr %u\n", addr);
	//printf("mem_page_get old log page tag: %u \n", mem->bound_logical_page_tag);

	//printf("tag : %u \n", mem->bound_logical_page_tag);
	uint32_t index, tag;
	struct mem_page_t *prev, *page;

	tag = addr & ~(MEM_PAGESIZE - 1);

	// Checking entry in TLB.
	if (isa_ctx) {
		page = tlb_lookup (tag, isa_ctx->pid);
		if (page!=NULL) {
			if (page->frame_id == -1)
				if (LOADINGPHASE != 1)
					handle_page_fault(mem, page);
			return page;
		}
	}

	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];
	prev = NULL;
	
	/* Look for page */
	while (page && page->tag != tag) {
		prev = page;
		page = page->next;
	}
	
	/* Place page into list head */
	if (prev && page) {
		prev->next = page->next;
		page->next = mem->pages[index];
		mem->pages[index] = page;
	}

	if(page==NULL){
		//printf("Might be allocating page. \n");
	}
	//else printf("mem_page_get_page frame id : %d", page->frame_id);
	//before returning make sure page has ram frame allocated
	if(page!= NULL && page->frame_id == -1){
		//printf("Page fault\n");
		if (LOADINGPHASE != 1)
			handle_page_fault(mem, page);
	}
	
	// Adding entry to TLB
	if (isa_ctx)
		if (page!=NULL)
			add_tlb_entry(page, isa_ctx->pid);

	/* Return found page */
	return page;	 
}


/* Return the memory page following addr in the current memory map. This function
 * is useful to reconstruct consecutive ranges of mapped pages. */
struct mem_page_t *mem_page_get_next(struct mem_t *mem, uint32_t addr)
{
	uint32_t tag, index, mintag;
	struct mem_page_t *prev, *page, *minpage;

	/* Get tag of the page just following addr */
	tag = (addr + MEM_PAGESIZE) & ~(MEM_PAGESIZE - 1);
	if (!tag)
		return NULL;
	index = (tag >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	page = mem->pages[index];
	prev = NULL;

	/* Look for a page exactly following addr. If it is found, return it. */
	while (page && page->tag != tag) {
		prev = page;
		page = page->next;
	}
	if (page)
		return page;
	
	/* Page following addr is not found, so check all memory pages to find
	 * the one with the lowest tag following addr. */
	mintag = 0xffffffff;
	minpage = NULL;
	for (index = 0; index < MEM_PAGE_COUNT; index++) {
		for (page = mem->pages[index]; page; page = page->next) {
			if (page->tag > tag && page->tag < mintag) {
				mintag = page->tag;
				minpage = page;
			}
		}
	}

	/* Return the found page (or NULL) */
	return minpage;
}


/* Create new mem page */
static struct mem_page_t *mem_page_create(struct mem_t *mem, uint32_t addr, int perm)
{

	//printf("mem_page_create : %u\n", addr);
	uint32_t index, tag;
	struct mem_page_t *page;

	tag = addr & ~(MEM_PAGESIZE - 1); //tag is actually (logical page number * page_size)
	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	
	/* Create new page */
	page = calloc(1, sizeof(struct mem_page_t));
	page->tag = tag;
	page->perm = perm;
	
	/* Insert in pages hash table */
	page->next = mem->pages[index];
	
	page->swap_page_no = allocate_page(); //allocate swap page(vmm.h)

	//ram_frame x = get_free_ram_frame();
	page->frame_id = -1;
	page->data = NULL;
	page->valid_bit = 0;
	page->dirty_bit = 0;
	page->isPinned = 0;

	mem->pages[index] = page;
	mem_mapped_space += MEM_PAGESIZE;
	mem_max_mapped_space = MAX(mem_max_mapped_space, mem_mapped_space);
	return page;
}


/* Free mem pages */
static void mem_page_free(struct mem_t *mem, uint32_t addr)
{
	//This means that this logical page will never be used again.
	//So we can free the swap page associated with it

	//NOTEIMP : we must also release the associated "allocated_frame" if any

	uint32_t index, tag;
	struct mem_page_t *prev, *page;
	struct mem_host_mapping_t *hm;
	
	tag = addr & ~(MEM_PAGESIZE - 1);
	index = (addr >> MEM_LOGPAGESIZE) % MEM_PAGE_COUNT;
	prev = NULL;

	/* Find page */
	page = mem->pages[index];
	while (page && page->tag != tag) {
		prev = page;
		page = page->next;
	}
	if (!page)
		return;
	
	/* If page belongs to a host mapping, release it if
	 * this is the last page allocated for it. */
	hm = page->host_mapping;
	if (hm) {
		assert(hm->pages > 0);
		assert(tag >= hm->addr && tag + MEM_PAGESIZE <= hm->addr + hm->size);
		hm->pages--;
		page->data = NULL;
		page->host_mapping = NULL;
		if (!hm->pages)
			mem_unmap_host(mem, hm->addr);
	}

	/* Free page */
	if (prev)
		prev->next = page->next;
	else
		mem->pages[index] = page->next;
	mem_mapped_space -= MEM_PAGESIZE;
	/*
	if (page->data){
		if(page->frame_id != -1){
			printf("mem_page_free - addr: %u\n", addr);
			//release_ram_frame(page->frame_id);//TODO don't release ram frame,
											// instead release the associated swap page
			
		}
		page->data = NULL; //now this ram frame no longer belongs to this
	}
	*/

	//Deallcate the swap page associated
	//Nothing to do with the ram frames. They will be released in void mem_free(struct mem_t *mem)
	deallocate_page(page->swap_page_no);
	release_allocated_frame(mem, tag);
	free(page);
}


/* Copy memory pages. All parameters must be multiple of the page size.
 * The pages in the source and destination interval must exist. */
void mem_copy(struct mem_t *mem, uint32_t dest, uint32_t src, int size)
{
	struct mem_page_t *page_dest, *page_src;

	/* Restrictions. No overlapping allowed. */
	assert(!(dest & (MEM_PAGESIZE-1)));
	assert(!(src & (MEM_PAGESIZE-1)));
	assert(!(size & (MEM_PAGESIZE-1)));
	if ((src < dest && src + size > dest) ||
		(dest < src && dest + size > src))
		fatal("mem_copy: cannot copy overlapping regions");
	
	/* Copy */
	while (size > 0) {
		
		/* Get source and destination pages */
		page_src = mem_page_get(mem, src);
		assert(page_src);
		unsigned char * temp_buf = malloc(MEM_PAGESIZE);
		memcpy(temp_buf, page_src->data, MEM_PAGESIZE);
		

		page_dest = mem_page_get(mem, dest);
		assert(page_dest);
		
		/* Different actions depending on whether source and
		 * destination page data are allocated. */
		if (temp_buf) {
			if (!page_dest->data)//Now this will not happen
								//(as ram frame is always allocated during creating of page)
				page_dest->data = malloc(MEM_PAGESIZE);
			memcpy(page_dest->data, temp_buf, MEM_PAGESIZE);
		} else {
			if (page_dest->data)
				memset(page_dest->data, 0, MEM_PAGESIZE);
		}

		/* Advance pointers */
		src += MEM_PAGESIZE;
		dest += MEM_PAGESIZE;
		size -= MEM_PAGESIZE;
	}
}


/* Return the buffer corresponding to address 'addr' in the simulated
 * mem. The returned buffer is null if addr+size exceeds the page
 * boundaries. */
void *mem_get_buffer(struct mem_t *mem, uint32_t addr, int size,
	enum mem_access_enum access)
{
	struct mem_page_t *page;
	uint32_t offset;

	/* Get page offset and check page bounds */
	offset = addr & (MEM_PAGESIZE - 1);
	if (offset + size > MEM_PAGESIZE)
		return NULL;
	
	/* Look for page */
	page = mem_page_get(mem, addr);
	if (!page)
		return NULL;
	
	/* Check page permissions */
	if ((page->perm & access) != access && mem->safe)
		fatal("mem_get_buffer: permission denied at 0x%x", addr);
	
	/* Allocate and initialize page data if it does not exist yet. */
	if (!page->data) //also this won't happen now as ram frame is alloc during page creation
		page->data = calloc(1, MEM_PAGESIZE);
	
	/* Return pointer to page data */
	return page->data + offset;
}


/* Access memory without exceeding page boundaries. */
static void mem_access_page_boundary(struct mem_t *mem, uint32_t addr,
	int size, void *buf, enum mem_access_enum access)
{
	//printf("mem access page boundary : addr : %u\n", addr);
	struct mem_page_t *page;
	uint32_t offset;

	/* Find memory page and compute offset. */
	page = mem_page_get(mem, addr);
	offset = addr & (MEM_PAGESIZE - 1);
	assert(offset + size <= MEM_PAGESIZE);

	/* On nonexistent page, raise segmentation fault in safe mode,
	 * or create page with full privileges for writes in unsafe mode. */
	if (!page) {
		if (mem->safe)
			fatal("illegal access at 0x%x: page not allocated", addr);
		if (access == mem_access_read || access == mem_access_exec) {
			memset(buf, 0, size);
			return;
		}
		if (access == mem_access_write || access == mem_access_init)
			page = mem_page_create(mem, addr, mem_access_read |
				mem_access_write | mem_access_exec |
				mem_access_init);
	}
	assert(page);

	/* If it is a write access, set the 'modified' flag in the page
	 * attributes (perm). This is not done for 'initialize' access. */
	if (access == mem_access_write)
		page->perm |= mem_access_modif;

	/* Check permissions in safe mode */
	if (mem->safe && (page->perm & access) != access){
		//fatal("mem_access: permission denied at 0x%x", addr);
            raise(SIGSEGV);
        }

	/* Read/execute access */
	if (access == mem_access_read || access == mem_access_exec) {
		if (page->data)
			memcpy(buf, page->data + offset, size);
		else
			memset(buf, 0, size);
		return;
	}

	/* Write/initialize access */
	if (access == mem_access_write || access == mem_access_init) {
		if (!page->data)
			page->data = calloc(1, MEM_PAGESIZE);
		memcpy(page->data + offset, buf, size);
		page->dirty_bit=1;
		return;
	}

	/* Shouldn't get here. */
	abort();
}

/* Access memory without exceeding page boundaries. 
	used only during loading of pages into swap 
*/
static void mem_access_page_boundary_swap(struct mem_t *mem, uint32_t addr,
	int size, void *buf, enum mem_access_enum access)
{
	if(access != mem_access_init){
		printf("Access should be mem_access_init\n");
		exit(0);
	}

	//access = mem_access_init
	struct mem_page_t *page;
	uint32_t offset;

	/* Find memory page and compute offset. */
	page = mem_page_get(mem, addr);
	offset = addr & (MEM_PAGESIZE - 1);
	assert(offset + size <= MEM_PAGESIZE);

	/* On nonexistent page, raise segmentation fault in safe mode,
	 * or create page with full privileges for writes in unsafe mode. */
	if (!page) {
		printf("mem_acc page boundaries (swap) : Should not come here as mem map called before\n" );
		//THIS won't happen as mem_map is called before calling this
		if (mem->safe)
			fatal("illegal access at 0x%x: page not allocated", addr);
		if (access == mem_access_write || access == mem_access_init)
			page = mem_page_create(mem, addr, mem_access_read |
				mem_access_write | mem_access_exec |
				mem_access_init);
	}
	assert(page);

	/* Check permissions in safe mode */
	if (mem->safe && (page->perm & access) != access){
		//fatal("mem_access: permission denied at 0x%x", addr);
            raise(SIGSEGV);
        }


   	uint32_t disk_page = page->swap_page_no;
   	uint32_t disk_page_start = (disk_page << MEM_LOGPAGESIZE);
   	uint32_t disk_addr = disk_page_start + offset;
	/* Write/initialize access */
	if (access == mem_access_write || access == mem_access_init) {
		FILE *fp = fopen("Sim_disk", "rb+");
	    if(fp == NULL){
	        printf("Couldn't open HardDisk access swap boundary");
	        exit(0);
	    }
	    fseek(fp, disk_addr, SEEK_SET);
	    fwrite(buf, size, 1, fp);
	    fclose(fp);
		return;
	}

	/* Shouldn't get here. */
	abort();
}

/* Access mem at address 'addr'.
 * This access can cross page boundaries. */
void mem_access(struct mem_t *mem, uint32_t addr, int size, void *buf,
	enum mem_access_enum access)
{
	uint32_t offset;
	int chunksize;

	mem->last_address = addr;
	while (size) {
		offset = addr & (MEM_PAGESIZE - 1);
		chunksize = MIN(size, MEM_PAGESIZE - offset);
		//printf("Inside mem_access\n");
		mem_access_page_boundary(mem, addr, chunksize, buf, access);

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}

/* Access mem at address 'addr'.
 * This access can cross page boundaries. */
void mem_access_swap(struct mem_t *mem, uint32_t addr, int size, void *buf,
	enum mem_access_enum access)
{
	uint32_t offset;
	int chunksize;

	mem->last_address = addr;
	while (size) {
		offset = addr & (MEM_PAGESIZE - 1);
		chunksize = MIN(size, MEM_PAGESIZE - offset);
		//system("ls -l Sim_disk");

		mem_access_page_boundary_swap(mem, addr, chunksize, buf, access);
		//system("ls -l Sim_disk");

		size -= chunksize;
		buf += chunksize;
		addr += chunksize;
	}
}


/* Creation and destruction */
struct mem_t *mem_create()
{
	struct mem_t *mem;
	mem = calloc(1, sizeof(struct mem_t));
	mem->sharing = 1;
	mem->safe = mem_safe_mode;

	mem->num_frames_allocated = 0;
	mem->num_pages_pinned = 0;
	return mem;
}


void mem_free(struct mem_t *mem)
{
	int i;
	
	/* Free pages */
	for (i = 0; i < MEM_PAGE_COUNT; i++)
		while (mem->pages[i])
			mem_page_free(mem, mem->pages[i]->tag);

	/* This must have released all host mappings.
	 * Now, free memory structure. */
	assert(!mem->host_mapping_list);

	//Free the allocated ram frames
	release_all_allocated_frames_to_system(mem);

	//assert that both queue and alloc_frame list are NULL
	if(mem->allocated_frames_head != NULL) printf("allocated_frames_head should be NULL at end of mem_free()\n");
	if(mem->fifo_queue_head != NULL) printf("fifo_queue_head should be NULL at end of mem_free()\n");
	free(mem);
}


/* This function finds a free memory region to allocate 'size' bytes
 * starting at address 'addr'. */
uint32_t mem_map_space(struct mem_t *mem, uint32_t addr, int size)
{

	uint32_t tag_start, tag_end;

	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag_start = addr;
	tag_end = addr;
	for (;;) {

		/* Address space overflow */
		if (!tag_end)
			return (uint32_t) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_end)) {
			tag_end += MEM_PAGESIZE;
			tag_start = tag_end;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGESIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGESIZE < size);
		
		/* we have a new free page */
		tag_end += MEM_PAGESIZE;
	}


	/* Return the start of the free space */
	return tag_start;
}


uint32_t mem_map_space_down(struct mem_t *mem, uint32_t addr, int size)
{
	uint32_t tag_start, tag_end;

	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag_start = addr;
	tag_end = addr;
	for (;;) {

		/* Address space overflow */
		if (!tag_start)
			return (uint32_t) -1;
		
		/* Not enough free pages in current region */
		if (mem_page_get(mem, tag_start)) {
			tag_start -= MEM_PAGESIZE;
			tag_end = tag_start;
			continue;
		}
		
		/* Enough free pages */
		if (tag_end - tag_start + MEM_PAGESIZE == size)
			break;
		assert(tag_end - tag_start + MEM_PAGESIZE < size);
		
		/* we have a new free page */
		tag_start -= MEM_PAGESIZE;
	}

	/* Return the start of the free space */
	return tag_start;
}


/* Allocate (if not already allocated) all necessary memory pages to
 * access 'size' bytes at 'addr'. These two fields do not need to be
 * aligned to page boundaries.
 * If some page already exists, add permissions. */
void mem_map(struct mem_t *mem, uint32_t addr, int size,
	enum mem_access_enum perm)
{
	uint32_t tag1, tag2, tag;
	struct mem_page_t *page;

	/* Calculate page boundaries */
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE) {
		page = mem_page_get(mem, tag);
		if (!page)
			page = mem_page_create(mem, tag, perm);
		page->perm |= perm;
	}
}


/* Deallocate memory pages. The addr and size parameters must be both
 * multiple of the page size.
 * If some page was not allocated, the corresponding address range is skipped.
 * If a host mapping is caught in the range, it is deallocated with a call
 * to 'mem_unmap_host'. */
void mem_unmap(struct mem_t *mem, uint32_t addr, int size)
{
	uint32_t tag1, tag2, tag;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Deallocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE)
		mem_page_free(mem, tag);
}


/* Map guest pages with the data allocated by a host 'mmap' call.
 * When this space is allocated with 'mem_unmap', the host memory
 * will be freed with a host call to 'munmap'.
 * Guest pages must already exist.
 * Both 'addr' and 'size' must be a multiple of the page size. */
void mem_map_host(struct mem_t *mem, struct fd_t *fd, uint32_t addr, int size,
	enum mem_access_enum perm, void *host_ptr)
{

	printf("inside mem map host\n");
	uint32_t ptr;
	struct mem_page_t *page;
	struct mem_host_mapping_t *hm;

	/* Check restrictions */
	if (addr & ~MEM_PAGEMASK)
		fatal("mem_map_host: 'addr' not a multiple of page size");
	if (size & ~MEM_PAGEMASK)
		fatal("mem_map_host: 'size' not a multiple of page size");
	
	/* Create host mapping, and insert it into the list head. */
	hm = calloc(1, sizeof(struct mem_host_mapping_t));
	hm->host_ptr = host_ptr;
	hm->addr = addr;
	hm->size = size;
	hm->next = mem->host_mapping_list;
	strncpy(hm->path, fd->path, MAX_PATH_SIZE);
	mem->host_mapping_list = hm;
	syscall_debug("  host mapping created for '%s'\n", hm->path);
	
	/* Make page data point to new data */
	for (ptr = addr; ptr < addr + size; ptr += MEM_PAGESIZE) {
		page = mem_page_get(mem, ptr);
		if (!page)
			fatal("mem_map_host: requested range not allocated");

		/* It is not allowed that the page belong to a previous host
		 * mapping. If so, it should have been unmapped before. */
		if (page->host_mapping)
			fatal("mem_map_host: cannot overwrite a previous host mapping");

		/* If page is pointing to some data, overwrite it */
		if (page->data){
			release_ram_frame(page->frame_id); //TODO dont do this, just free the binding of page in global ram
		}

		/* Create host mapping */
		page->frame_id = -1; //because now page->data will point to some host ptr(see below)
		page->host_mapping = hm;
		page->data = ptr - addr + host_ptr;
		hm->pages++;
	}
}


/* Deallocate host mapping starting at address 'addr'.
 * A host call to 'munmap' is performed to unmap host space. */
void mem_unmap_host(struct mem_t *mem, uint32_t addr)
{
	printf("inside mem unmap host\n");
	int ret;
	struct mem_host_mapping_t *hm, *hmprev;

	/* Locate host mapping in the list */
	hmprev = NULL;
	hm = mem->host_mapping_list;
	while (hm && hm->addr != addr) {
		hmprev = hm;
		hm = hm->next;
	}

	/* Remove it from the list */
	assert(hm);
	if (hmprev)
		hmprev->next = hm->next;
	else
		mem->host_mapping_list = hm->next;
	
	/* Perform host call to 'munmap' */
	ret = munmap(hm->host_ptr, hm->size);
	if (ret < 0)
		fatal("mem_unmap_host: host call 'munmap' failed");
	
	/* Free host mapping */
	syscall_debug("  host mapping removed for '%s'\n", hm->path);
	free(hm);
}


/* Assign protection attributes to pages */
void mem_protect(struct mem_t *mem, uint32_t addr, int size, enum mem_access_enum perm)
{
	uint32_t tag1, tag2, tag;
	struct mem_page_t *page;
	int prot, err;

	/* Calculate page boundaries */
	assert(!(addr & (MEM_PAGESIZE - 1)));
	assert(!(size & (MEM_PAGESIZE - 1)));
	tag1 = addr & ~(MEM_PAGESIZE-1);
	tag2 = (addr + size - 1) & ~(MEM_PAGESIZE-1);

	/* Allocate pages */
	for (tag = tag1; tag <= tag2; tag += MEM_PAGESIZE) {
		page = mem_page_get(mem, tag);
		if (!page)
			continue;

		/* Set page new protection flags */
		page->perm = perm;

		/* If the page corresponds to a host mapping, host page must
		 * update its permissions, too */
		if (page->host_mapping) {
			prot = (perm & mem_access_read ? PROT_READ : 0) |
				(perm & mem_access_write ? PROT_WRITE : 0) |
				(perm & mem_access_exec ? PROT_EXEC : 0);
			err = mprotect(page->data, MEM_PAGESIZE, prot);
			if (err < 0)
				fatal("mem_protect: host call to 'mprotect' failed");
		}
	}
}


void mem_write_string(struct mem_t *mem, uint32_t addr, char *str)
{
	mem_access(mem, addr, strlen(str) + 1, str, mem_access_write);
}


/* Read a string from memory and return the length of the read string.
 * If the return length is equal to max_size, it means that the string did not
 * fit in the destination buffer. */
int mem_read_string(struct mem_t *mem, uint32_t addr, int size, char *str)
{
	int i;
	for (i = 0; i < size; i++) {
		mem_access(mem, addr + i, 1, str + i, mem_access_read);
		if (!str[i])
			break;
	}
	return i;
}


void mem_zero(struct mem_t *mem, uint32_t addr, int size)
{
	unsigned char zero = 0;
	while (size--)
		mem_access(mem, addr++, 0, &zero, mem_access_write);
}


void mem_dump(struct mem_t *mem, char *filename, uint32_t start, uint32_t end) //NEVERCALLED
{
	FILE *f;
	uint32_t size;
	uint8_t buf[MEM_PAGESIZE];

	f = fopen(filename, "wb");
	if (!f)
		fatal("mem_dump: cannot open file '%s'", filename);
	
	/* Set unsafe mode and dump */
	mem->safe = 0;
	while (start < end) {
		size = MIN(MEM_PAGESIZE, end - start);
		mem_access(mem, start, size, buf, mem_access_read);
		fwrite(buf, size, 1, f);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = mem_safe_mode;
	fclose(f);
}


void mem_load(struct mem_t *mem, char *filename, uint32_t start)//NEVERCALLED
{
	FILE *f;
	uint32_t size;
	uint8_t buf[MEM_PAGESIZE];
	
	f = fopen(filename, "rb");
	if (!f)
		fatal("mem_load: cannot open file '%s'", filename);
	
	/* Set unsafe mode and load */
	mem->safe = 0;
	for (;;) {
		size = fread(buf, 1, MEM_PAGESIZE, f);
		if (!size)
			break;
		mem_access(mem, start, size, buf, mem_access_write);
		start += size;
	}

	/* Restore safe mode */
	mem->safe = mem_safe_mode;
	fclose(f);
}

