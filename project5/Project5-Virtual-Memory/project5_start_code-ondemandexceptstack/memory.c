/* Author(s): <Your name here>
 * Implementation of the memory manager for the kernel.
 */

#include "common.h"
#include "interrupt.h"
#include "kernel.h"
#include "memory.h"
#include "printf.h"
#include "scheduler.h"
#include "util.h"
#include "sync.h"

#define MEM_START 0x00908000

extern void tlb_flush();

/* Static global variables */
// Keep track of all pages: their vaddr, status, and other properties
static page_map_entry_t page_map[ PAGEABLE_PAGES ];

// other global variables...
static lock_t page_fault_lock;

/* TODO: Returns physical address of page number i */
uint32_t page_paddr( int i ) {
    return MEM_START + i*PAGE_SIZE;
}
/* TODO: Returns virtual address (in kernel) of page number i */
uint32_t page_vaddr( int i ) {
    return MEM_START + i*PAGE_SIZE + 0xa0000000;
}

/* get the physical address from virtual address (in kernel) */
uint32_t va2pa( uint32_t va ) {
    return (uint32_t) va - 0xa0000000;
}

/* get the virtual address (in kernel) from physical address */
uint32_t pa2va( uint32_t pa ) {
    return (uint32_t) pa + 0xa0000000;
}


uint32_t get_page_idx(uint32_t vaddr){
	return (vaddr & PAGE_MASK) >> 12;
}


// TODO: insert page table entry to page table
void insert_page_table_entry( uint32_t *table, uint32_t vaddr, uint32_t paddr,
                              uint32_t flag, uint32_t pid ) {
    // insert entry
	table[get_page_idx(vaddr)] = (paddr & PAGE_MASK) | (flag & FLAG_MASK) | ((pid<<4) & PID_MASK);
	if (flag){
	//printf(19,1,"INFO in insert page table entry");
	//printf(20,1,"The vaddr:0x%x\tThe paddr:0x%x",vaddr,paddr);
	//printf(21,1,"write place 0x%x",table+get_page_idx(vaddr));
	//printf(22,1,"The PTE:0x%x",table[get_page_idx(vaddr)]);
	}
	printf(23,1,"return from insert");
    tlb_flush(vaddr & 0xffffe000 | pid & 0xff);
	
	return;
}

/* TODO: Allocate a page. Return page index in the page_map directory.
 *
 * Marks page as pinned if pinned == TRUE.
 * Swap out a page if no space is available.
 */
int page_alloc( int pinned ) {
 
    // code here
    int free_index;
	for (free_index = 0; free_index < PAGEABLE_PAGES; free_index++){
		if (page_map[free_index].used == FALSE)
			break;
	}
	if (free_index >= PAGEABLE_PAGES){
		
	}
	page_map[free_index].dirty = FALSE;
	page_map[free_index].pid = current_running->pid;
	page_map[free_index].pinned = pinned;
	page_map[free_index].used = TRUE;
   
    ASSERT( free_index < PAGEABLE_PAGES );
	bzero((char*)page_vaddr(free_index), PAGE_SIZE);
	return free_index;
}

/* TODO: 
 * This method is only called once by _start() in kernel.c
 */
uint32_t init_memory( void ) {
    
    // initialize all pageable pages to a default state
    int i;
   	lock_init(&page_fault_lock);
    for (i = 0; i < PAGEABLE_PAGES; i++){
		page_map[i].dirty = FALSE;
		page_map[i].pid = -1;
		page_map[i].pinned = FALSE;
		page_map[i].used = FALSE;
	}
	return 0;
}

/* TODO: 
 * 
 */
uint32_t *setup_page_table( int pid ) {
    uint32_t *page_table;
	
    // alloc page for page table 
	int page_index = page_alloc(TRUE);
	page_table = (uint32_t *)page_vaddr(page_index);
	page_map[page_index].pid = pid;
	
    // initialize PTE and insert several entries into page tables using insert_page_table_entry
    
	int proc_pages = (pcb[pid].size - 1) / PAGE_SIZE + 1;
	int i;
	if (on_demand == TRUE){
		for (i = 0; i < proc_pages; i++){
			insert_page_table_entry(page_table, pcb[pid].entry_point + i*PAGE_SIZE ,0,0,pid);
		}
		//page_index = page_alloc(TRUE);
		//insert_page_table_entry(page_table, pcb[pid].user_tf.regs[29]-1,page_paddr(page_index),PE_V|PE_D,pid);
	}
	else{
		for (i = 0; i < proc_pages; i++){
			page_index = page_alloc(FALSE);
			bcopy((char *)(pcb[pid].loc+i*PAGE_SIZE),(char *) page_vaddr(page_index),PAGE_SIZE);
			insert_page_table_entry(page_table, pcb[pid].entry_point + i*PAGE_SIZE,page_paddr(page_index),PE_V|PE_D,pid);
			
		}
		page_index = page_alloc(TRUE);
		insert_page_table_entry(page_table, pcb[pid].user_tf.regs[29]-1,page_paddr(page_index),PE_V|PE_D,pid);
	}
    return page_table;
}

void do_tlb_miss() {
	current_running->tlb_miss++;
}


void temp_print(uint32_t a,int num){
	printf(23+num,1,"0x%x",a);
	return;
}


void print_tlb(uint32_t i,uint32_t entryhi, uint32_t entrylo0, uint32_t entrylo1){
	printf(i+1,40,"0x%x \t 0x%x \t 0x%x",entryhi,entrylo0,entrylo1);
}


void create_pte(uint32_t vaddr, int pid) {
    return;
}


void do_page_fault(uint32_t vaddr){
	printf(7,1,"enter do_page_fault,now pid is %d",current_running->pid);
	printf(8,1,"the vaddr is 0x%x",vaddr);

	current_running->nested_count++;
	lock_acquire(&page_fault_lock);
	current_running->page_fault++;
	
	int i;
	if (vaddr > 0x200000)
	{
		i = page_alloc(TRUE);
		printf(16,1,"alloc phy page %d",i);
		insert_page_table_entry(current_page_table,vaddr,page_paddr(i),PE_V|PE_D,current_running->pid);
	}
	else 
	{
		i = page_alloc(FALSE);
		printf(16,1,"alloc phy page %d",i);
		bcopy((char *)(current_running->loc+(get_page_idx(vaddr)<<12)-current_running->entry_point),(char *) page_vaddr(i),PAGE_SIZE);
		insert_page_table_entry(current_page_table,vaddr,page_paddr(i),PE_V|PE_D,current_running->pid);
	}
	lock_release(&page_fault_lock);
	current_running->nested_count--;
	printf(17,1,"leave do_page_fault");
}
