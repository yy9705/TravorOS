#include <kernel/memory.h>
#include <bitset32.h>
#include <kernel/utils.h>
#include <kernel/bsod.h>
#include <asm/string.h>
#include <drivers/screen.h>
#include <cpu/isr.h>
#include <stdio.h>

extern bitset_t *frame_bitset;
extern size_t bitset_size;
size_t *kernel_directory;
page_t *kernel_table;
page_t *kernel_heap;
page_t *liballoc_heap;

static void page_fault(registers_t regs)
{
	size_t faulting_address;
	int present,rw,us,reserved,id;
	present=regs.err_code & 1;	// Page not present
	rw=regs.err_code & 0x2;			// Write protection?
	us=regs.err_code & 0x4;			// User-mode?
	reserved=regs.err_code & 0x8;	// Overwritten CPU-reserved bits of page entry?
	id=regs.err_code & 0x10;		// Caused by an instruction fetch?
	__asm__("movl %%cr2,%0":"=r" (faulting_address));
	char description[512];
	sprintf(description,"Page fault occurs while %s address 0x%x\nPage attributes: %s%s%s%s",
		rw?"writing":"reading",
		faulting_address,
		present?"\0":"not-present ",
		us?"user-mode ":"\0",
		reserved?"cpu-reserved ":"\0",
		id?"instruction-fetch":"\0"
	);
	bsod_enter("Page Fault",description,&regs);
}

void init_paging(void)
{
	assert(sizeof(page_t) == 4);	// Make sure the page union is set up properly
	memset(frame_bitset,0,bitset_size*sizeof(size_t));
	kprint("Initializing Paging...\n");
	frame_bitset=(bitset_t*)kmalloc_a(bitset_size);
	kernel_directory=(size_t*)kmalloc_a(1024*sizeof(size_t));
	kernel_table=(page_t*)kmalloc_a(1024*sizeof(page_t));
	kernel_heap=(page_t*)kmalloc_a(1024*sizeof(page_t));
	liballoc_heap=(page_t*)kmalloc_a(1024*sizeof(page_t));
	// TODO: Initialize the page directory
	size_t i;
	for(i=0;i<1024;i++){
		/*
		* Set the following flags to the pages:
		* Superwisor: Only kernel-mode can access them
		* Write Enabled: It can be both read from and written to
		* Not present: The page table is not present
		*/
		kernel_directory[i]=0x00000002;
	}
	/* Identity map the first 8MB of the physical memory */
	for(i=0;i<1024;i++){
		map_frame(kernel_table+i,i,0,1);
		set_frame(i);
	}
	for(i=0;i<1024;i++){
		map_frame(kernel_heap+i,i+1024,0,1);
		set_frame(i+1024);
	}
	for(i=0;i<1024;i++){
		liballoc_heap[i].present=0;	// Make sure they are not present
	}
	kernel_table[3].val=3*0x1000;	// Create a not-present page
	kernel_directory[0]=((size_t)kernel_table) | 3;		/* 0 - 4MB:  Kernel space */
	kernel_directory[1]=((size_t)kernel_heap) | 3;		/* 4 - 8MB:  Kernel heap */
	kernel_directory[2]=((size_t)liballoc_heap) | 3;	/* 8 - 12MB: liballoc heap */
	register_interrupt_handler(0xE,&page_fault);
	load_page_directory(kernel_directory);
	enable_paging();
	ktrace("SUCCESS");
}

page_t *get_page(void *address,char create,size_t *pgdir)
{
	size_t addr=(size_t)address;
	addr/=0x1000;
	size_t table_index=addr/1024;
	if(pgdir[table_index]){
		return (void*)(pgdir[table_index]+addr%1024);
	}else if(create){
		size_t tmp=(size_t)kmalloc_a(1024*sizeof(page_t));
		memset((void*)tmp,0,0x1000);
		pgdir[table_index]=tmp | 0x7;	// PRESENT, RW, US.
		return (void*)(pgdir[table_index]+addr%1024);
	}else{
		return NULL;
	}
}

/* This function helps me integrate liballoc into my kernel */
void *get_free_page(size_t *pgdir)
{
	size_t tb_index;
	size_t pg_index;
}
