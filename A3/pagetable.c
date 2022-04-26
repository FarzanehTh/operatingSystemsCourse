#include <assert.h>
#include <string.h> 
#include "sim.h"
#include "pagetable.h"

// The top-level page table (also known as the 'page directory')
pgdir_entry_t pgdir[PTRS_PER_PGDIR]; 

// Counters for various events.
// Your code must increment these when the related events occur.
int hit_count = 0;
int miss_count = 0;
int ref_count = 0;
int evict_clean_count = 0;
int evict_dirty_count = 0;

/*
 * Allocates a frame to be used for the virtual page represented by p.
 * If all frames are in use, calls the replacement algorithm's evict_fcn to
 * select a victim frame.  Writes victim to swap if needed, and updates 
 * pagetable entry for victim to indicate that virtual page is no longer in
 * (simulated) physical memory.
 *
 * Counters for evictions should be updated appropriately in this function.
 */
static int allocate_frame(pgtbl_entry_t *p)
{

    /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

    // Record information for virtual page that will now be stored in frame
	coremap[frame].in_use = 1;
	coremap[frame].pte = p;

	return frame;
}

/*
 * Initializes the top-level pagetable.
 * This function is called once at the start of the simulation.
 * For the simulation, there is a single "process" whose reference trace is 
 * being simulated, so there is just one top-level page table (page directory).
 * To keep things simple, we use a global array of 'page directory entries'.
 *
 * In a real OS, each process would have its own page directory, which would
 * need to be allocated and initialized as part of process creation.
 */
void init_pagetable(void)
{
	// Set all entries in top-level pagetable to 0, which ensures valid
	// bits are all 0 initially.
	for (int i = 0; i < PTRS_PER_PGDIR; i++) {
		pgdir[i].pde = 0;
	}
}

// For simulation, we get second-level pagetables from ordinary memory
static pgdir_entry_t init_second_level(void)
{
	pgtbl_entry_t *pgtbl;
	// Allocating aligned memory ensures the low bits in the pointer must
	// be zero, so we can use them to store our status bits, like PG_VALID

    /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	return new_entry;
}

/* 
 * Initializes the content of a (simulated) physical memory frame when it 
 * is first allocated for some virtual address.  Just like in a real OS,
 * we fill the frame with zero's to prevent leaking information across
 * pages. 
 * 
 * In our simulation, we also store the the virtual address itself in the 
 * page frame to help with error checking.
 *
 */
static void init_frame(int frame, addr_t vaddr)
{
	// Calculate pointer to start of frame in (simulated) physical memory
	char *mem_ptr = &physmem[frame * SIMPAGESIZE];
	// Calculate pointer to location in page where we keep the vaddr
	addr_t *vaddr_ptr = (addr_t *)(mem_ptr + sizeof(int));

    /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
}

/*
 * Locate the physical frame number for the given vaddr using the page table. (i.e: do the address translation)
 *
 * If the entry is invalid and not on swap, then this is the first reference 
 * to the page and a (simulated) physical frame should be allocated and 
 * initialized (using init_frame).  
 *
 * If the entry is invalid and on swap, then a (simulated) physical frame
 * should be allocated and filled by reading the page data from swap.
 *
 * Counters for hit, miss and reference events should be incremented in
 * this function.
 */
char *find_physpage(addr_t vaddr, char type)
{
	pgtbl_entry_t *p = NULL; // pointer to the full page table entry for vaddr
	unsigned idx = PGDIR_INDEX(vaddr); // get index into page directory

	// IMPLEMENTATION NEEDED
	// Use top-level page directory to get pointer to 2nd-level page table
	pgdir_entry_t pgdir_entry = pgdir[idx]; 
	// if the pde is not valid, init a 2nd level page table
	if (!(pgdir_entry.pde & PG_VALID)){
		pgdir_entry = init_second_level();
		pgdir[idx] = pgdir_entry;
	}

	// Use vaddr to get index into 2nd-level page table and initialize 'p' 
	p = (pgtbl_entry_t *)(pgdir_entry.pde & PAGE_MASK);
	unsigned pte_idx = PGTBL_INDEX(vaddr);
	unsigned int p_frame = p[pte_idx].frame;

	// Check if p is valid or not, on swap or not, and handle appropriately
	// (Note that the first acess to a page will be marked DIRTY.)

	// case 1: if it is a new page entry for which there is no physical memory frame
	if ((!(p_frame & PG_VALID) && !(p_frame & PG_ONSWAP)))
	{
		// allocate a new frame in physical memory for it
		unsigned int frame = allocate_frame(&(p[pte_idx]));
		init_frame(frame, vaddr);
		p[pte_idx].frame = frame << PAGE_SHIFT;
		p[pte_idx].frame = p[pte_idx].frame | PG_DIRTY;
		miss_count++;
	}
	else if (!(p_frame & PG_VALID) && (p_frame & PG_ONSWAP))
	// case 2: if it is a invalid page AND it is in swap file
	{ 
		// swap the page in
		unsigned int frame = allocate_frame(&(p[pte_idx]));
		init_frame(frame, vaddr);
		p[pte_idx].frame = frame << PAGE_SHIFT;
		swap_pagein(p[pte_idx].frame >> PAGE_SHIFT, p[pte_idx].swap_off);
		miss_count++;
	}else 
	{
		// case 3: if it is a valid page which is currently in physical memory
		hit_count++;
	}
	

	// Make sure that p is marked valid and referenced.
	p[pte_idx].frame = p[pte_idx].frame | (PG_VALID | PG_REF);

	if (type == 'S' || type == 'M')
	{
		// mark it dirty if the access type indicates that the page will be written to
		p[pte_idx].frame = p[pte_idx].frame | PG_DIRTY;
	}

	ref_count++;

	// Call replacement algorithm's ref_fcn for this page
	ref_fcn(&p[pte_idx]);

	// Return pointer into (simulated) physical memory at start of frame
	return &physmem[(p[pte_idx].frame >> PAGE_SHIFT) * SIMPAGESIZE];
}

void print_pagetable(pgtbl_entry_t *pgtbl)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
}

void print_pagedirectory(void)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
}
