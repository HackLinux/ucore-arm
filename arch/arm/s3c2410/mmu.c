#include <mmu.h>
#include <types.h>

void
arch_enable_paging(uintptr_t pgdir) {
	asm (
		"mcr p15,0,%0,c2,c0,0\n"    /* set base address of page table*/
		"mvn r0,#0\n"                  
		"mcr p15,0,r0,c3,c0,0\n"    /* enable all region access*/

		"mov r0,#0x1\n"
		"mcr p15,0,r0,c1,c0,0\n"    /* set back to control register */
		"mov r0,r0\n"
		"mov r0,r0\n"
		"mov r0,r0\n"
		:
		: "r" (pgdir)
		:"r0"
	);
}
