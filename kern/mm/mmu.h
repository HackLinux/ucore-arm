#ifndef __KERN_MM_MMU_H__
#define __KERN_MM_MMU_H__
#include <types.h>
#include <arch_mmu.h>

// A linear address 'la' has a three-part structure as follows:
//
// +--------12--------+-----10-------+-+-------12----------+
// | Page Directory   | Page Table   | Offset within Page  |
// |      Index       |   Index      |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \----------- PPN(la) -----------/
//
// The PDX, PTX, PGOFF, and PPN macros decompose linear addresses as shown.
// To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
// use PGADDR(PDX(la), PTX(la), PGOFF(la)).

// page directory index
// XXX #define PDX(la) ((((uintptr_t)(la)) >> PDXSHIFT) & 0x3FF)
#define PDX(la) ((((uintptr_t)(la)) >> 20) & 0xFFF)

// page table index
// XXX #define PTX(la) ((((uintptr_t)(la)) >> PTXSHIFT) & 0x3FF)
#define PTX(la) ((((uintptr_t)(la)) >> 10) & 0x3FF)

// page number field of address
#define PPN(la) (((uintptr_t)(la)) >> PTXSHIFT)

// offset in page
#define PGOFF(la) (((uintptr_t)(la)) & 0xFFF)

// construct linear address from indexes and offset
#define PGADDR(d, t, o) ((uintptr_t)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// address in page table or page directory entry
#define PTE_ADDR(pte)   ((uintptr_t)(pte) & ~0xFFF)
#define PDE_ADDR(pde)   PTE_ADDR(pde)

/* page directory and page table constants */
#define NPDEENTRY       1024                    // page directory entries per page directory
#define NPTEENTRY       1024                    // page table entries per page table

#define PGSIZE          4096                    // bytes mapped by a page
#define PGSHIFT         12                      // log2(PGSIZE)
#define PTSIZE          (PGSIZE * NPTEENTRY)    // bytes mapped by a page directory entry
#define PTSHIFT         22                      // log2(PTSIZE)

#define PTXSHIFT        12                      // offset of PTX in a linear address
#define PDXSHIFT        22                      // offset of PDX in a linear address

void arch_enable_paging(uintptr_t pgdir);

#endif /* !__KERN_MM_MMU_H__ */

