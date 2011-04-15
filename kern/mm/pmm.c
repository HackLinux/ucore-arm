#include <types.h>
#include <stdio.h>
#include <string.h>
#include <memlayout.h>
#include <pmm.h>
#include <buddy_pmm.h>
#include <error.h>
#include <memmap.h>
#include <mmu.h>
#include <slab.h>

// virtual address of physicall page array
struct Page *pages;
// amount of physical memory (in pages)
size_t npage = 0;

// virtual address of boot-time page directory
pde_t *boot_pgdir = NULL;

// physical memory management
const struct pmm_manager *pmm_manager;

/* *
 * The page directory entry corresponding to the virtual address range
 * [VPT, VPT + PTSIZE) points to the page directory itself. Thus, the page
 * directory is treated as a page table as well as a page directory.
 *
 * One result of treating the page directory as a page table is that all PTEs
 * can be accessed though a "virtual page table" at virtual address VPT. And the
 * PTE for number n is stored in vpt[n].
 *
 * A second consequence is that the contents of the current page directory will
 * always available at virtual address PGADDR(PDX(VPT), PDX(VPT), 0), to which
 * vpd is set bellow.
 * */
pte_t * const vpt = (pte_t *)VPT;
pde_t * const vpd = (pde_t *)PGADDR(PDX(VPT), PDX(VPT), 0);


static void check_alloc_page(void);
static void check_pgdir(void);
static void check_boot_pgdir(void);

//init_pmm_manager - initialize a pmm_manager instance
static void
init_pmm_manager(void) {
    pmm_manager = &buddy_pmm_manager;
    cprintf("memory management: %s\n", pmm_manager->name);
    pmm_manager->init();
}

//init_memmap - call pmm->init_memmap to build Page struct for free memory  
static void
init_memmap(struct Page *base, size_t n) {
    pmm_manager->init_memmap(base, n);
}

//alloc_pages - call pmm->alloc_pages to allocate a continuous n*PAGESIZE memory 
struct Page *
alloc_pages(size_t n) {
    struct Page *page;
    unsigned long intr_flag;
    local_irq_save(intr_flag);
    {
        page = pmm_manager->alloc_pages(n);
    }
    local_irq_restore(intr_flag);
    return page;
}

//free_pages - call pmm->free_pages to free a continuous n*PAGESIZE memory 
void
free_pages(struct Page *base, size_t n) {
    unsigned long intr_flag;
    local_irq_save(intr_flag);
    {
        pmm_manager->free_pages(base, n);
    }
    local_irq_restore(intr_flag);
}

// nr_free_pages - call pmm->nr_free_pages to get the size (nr*PAGESIZE) 
// of current free memory
size_t
nr_free_pages(void) {
    size_t ret;
    unsigned long intr_flag;
    local_irq_save(intr_flag);
    {
        ret = pmm_manager->nr_free_pages();
    }
    local_irq_restore(intr_flag);
    return ret;
}

/* pmm_init - initialize the physical memory management */
static void
page_init(void) {
    int i;

    init_physical_memmap();

    // print memory map
    cprintf("mem_map:\n");
    for (i = 0; i < mem_map.nr_map; i ++) {
        uint64_t begin = mem_map.map[i].addr, end = begin + mem_map.map[i].size;
        cprintf("  memory: %08llx, [%08llx, %08llx], type = %d.\n",
                mem_map.map[i].size, begin, end - 1, mem_map.map[i].type);
    }

    // Find out max physical address for kernel.
    uint64_t maxpa = 0;
    for (i = 0; i < mem_map.nr_map; i ++) {
        uint64_t begin = mem_map.map[i].addr, end = begin + mem_map.map[i].size;
        if (mem_map.map[i].type == MEMMAP_MEMORY) {
            if (maxpa < end && begin < KERN_PHYSICAL_MEM_BASE + KMEMSIZE) {
                maxpa = end;
            }
        }
    }
    if (maxpa > KERN_PHYSICAL_MEM_BASE + KMEMSIZE) maxpa = KERN_PHYSICAL_MEM_BASE + KMEMSIZE;

    cprintf("maxpa: %08llx\n", maxpa);

    // Where kernel ends.
    extern char end[];

    // npage is the number of pages for kernel physical memory.
    // npage = (maxpa - KERN_PHYSICAL_MEM_BASE) / PGSIZE;
    npage = maxpa / PGSIZE;
    // Physical page table starts where kernel ends.
    pages = (struct Page *)ROUNDUP((void *)end, PGSIZE);

    // Set all pages to be reserved.
    for (i = 0; i < npage; i ++) {
        SetPageReserved(pages + i);
    }

    // freemem is the free space start.
    // i.e. free physical mem + mem for storing physical pages
    uintptr_t freemem = PADDR((uintptr_t)pages + sizeof(struct Page) * npage);
    cprintf("freemem: %08lx\n", freemem);

    // Initialize memmap for each memory region.
    for (i = 0; i < mem_map.nr_map; i ++) {
        uint64_t begin = mem_map.map[i].addr, end = begin + mem_map.map[i].size;
        if (mem_map.map[i].type == MEMMAP_MEMORY) {
            if (begin < freemem) {
                begin = freemem;
            }
            if (end > KERN_PHYSICAL_MEM_BASE + KMEMSIZE) {
                end = KERN_PHYSICAL_MEM_BASE + KMEMSIZE;
            }

            begin = ROUNDUP(begin, PGSIZE);
            end = ROUNDDOWN(end, PGSIZE);
            if (begin < end) {
                init_memmap(pa2page(begin), (end - begin) / PGSIZE);
            }
        }
    }
}

static void
enable_paging(void) {
    cprintf("enabling paging\n");
    arch_enable_paging((uintptr_t)boot_pgdir);
}

//boot_map_segment - setup&enable the paging mechanism
// parameters
//  la:   linear address of this memory need to map (after x86 segment map)
//  size: memory size
//  pa:   physical address of this memory
//  perm: permission of this memory  
static void
boot_map_segment(pde_t *pgdir, uintptr_t la, size_t size, uintptr_t pa, uint32_t perm) {
    assert(PGOFF(la) == PGOFF(pa));
    la = ROUNDDOWN(la, PGSIZE);
    pa = ROUNDDOWN(pa, PGSIZE);
    for (; size > 0; size -= 1<<10, la += 1<<10, pa += 1<<10) {
        pte_t *ptep = get_pte(pgdir, la, 1);
        assert(ptep != NULL);
        *ptep = pa | PTE_P | perm;
    }
}

//boot_alloc_page - allocate one page using pmm->alloc_pages(1) 
// return value: the kernel virtual address of this allocated page
//note: this function is used to get the memory for PDT(Page Directory Table)&PT(Page Table)
static void *
boot_alloc_page(void) {
    struct Page *p;
    // XXX size
    p = alloc_pages(8);
    if (p == NULL)
        panic("boot_alloc_page failed.\n");
    // XXX hacky to get an 16k align. need a better method.
    return (ROUNDUP(page2kva(p), 1<<14));
}

static void
check_alloc_page(void) {
    pmm_manager->check();
    cprintf("check_alloc_page() succeeded!\n");
}

//pmm_init - setup a pmm to manage physical memory, build PDT&PT to setup paging mechanism 
//         - check the correctness of pmm & paging mechanism, print PDT&PT
void
pmm_init(void) {
    // We need to alloc/free the physical memory (granularity is 4KB or other size). 
    // So a framework of physical memory manager (struct pmm_manager)is defined in pmm.h
    // 
    // First we should init a physical memory manager(pmm) based on the framework.
    // Then pmm can alloc/free the physical memory. 
    // Now the first_fit/best_fit/worst_fit/buddy_system pmm are available.
    init_pmm_manager();

    // detect physical memory space, reserve already used memory,
    // then use pmm->init_memmap to create free page list
    page_init();

    //use pmm->check to verify the correctness of the alloc/free function in a pmm
    check_alloc_page();

    // create boot_pgdir, an initial page directory(Page Directory Table, PDT)
    boot_pgdir = boot_alloc_page();
    cprintf("pgdir is at: %08lx\n", boot_pgdir);
    memset(boot_pgdir, 0, 4 * PGSIZE);

    // Kernel
    boot_map_segment(boot_pgdir, KERNBASE, KMEMSIZE, KERN_PHYSICAL_MEM_BASE, PTE_RW);
    // IO
    boot_map_segment(boot_pgdir, 0x48000000, 0x18000000, 0x48000000, PTE_RW);

    enable_paging();
    cprintf("paging enabled.\n");

    slab_init();

    cprintf("DONE!=====================\n"); return;
}

//get_pte - get pte and return the kernel virtual address of this pte for la
//        - if the PT contians this pte didn't exist, alloc a page for PT
// parameter:
//  pgdir:  the kernel virtual base address of PDT
//  la:     the linear address need to map
//  create: a logical value to decide if alloc a page for PT
// return vaule: the kernel virtual address of this pte
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
    pde_t *pdep = &pgdir[PDX(la)];
    if (!(*pdep & PDE_P)) {
        struct Page *page;
        if (!create || (page = alloc_page()) == NULL) {
            return NULL;
        }
        set_page_ref(page, 1);
        uintptr_t pa = page2pa(page);
        memset(KADDR(pa), 0, PGSIZE);
        *pdep = pa | PDE_FINE | PDE_P;
    }
    return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(la)];
}

//get_page - get related Page struct for linear address la using PDT pgdir
struct Page *
get_page(pde_t *pgdir, uintptr_t la, pte_t **ptep_store) {
    pte_t *ptep = get_pte(pgdir, la, 0);
    if (ptep_store != NULL) {
        *ptep_store = ptep;
    }
    if (ptep != NULL && *ptep & PTE_P) {
        return pa2page(*ptep);
    }
    return NULL;
}

//page_remove_pte - free an Page sturct which is related linear address la
//                - and clean(invalidate) pte which is related linear address la
//note: PT is changed, so the TLB need to be invalidate 
static inline void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
    if (*ptep & PTE_P) {
        struct Page *page = pte2page(*ptep);
        if (page_ref_dec(page) == 0) {
            free_page(page);
        }
        *ptep = 0;
        tlb_invalidate(pgdir, la);
    }
}

//page_remove - free an Page which is related linear address la and has an validated pte
void
page_remove(pde_t *pgdir, uintptr_t la) {
    pte_t *ptep = get_pte(pgdir, la, 0);
    if (ptep != NULL) {
        page_remove_pte(pgdir, la, ptep);
    }
}

//page_insert - build the map of phy addr of an Page with the linear addr la
// paramemters:
//  pgdir: the kernel virtual base address of PDT
//  page:  the Page which need to map
//  la:    the linear address need to map
//  perm:  the permission of this Page which is setted in related pte
// return value: always 0
//note: PT is changed, so the TLB need to be invalidate 
int
page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm) {
    pte_t *ptep = get_pte(pgdir, la, 1);
    if (ptep == NULL) {
        return -E_NO_MEM;
    }
    page_ref_inc(page);
    if (*ptep & PTE_P) {
        struct Page *p = pte2page(*ptep);
        if (p == page) {
            page_ref_dec(page);
        }
        else {
            page_remove_pte(pgdir, la, ptep);
        }
    }
    *ptep = page2pa(page) | PTE_P | perm;
    tlb_invalidate(pgdir, la);
    return 0;
}

void
tlb_invalidate(pde_t *pgdir, uintptr_t la) {
    asm(
    "mcr p15,0,r0,c8,c7,0;"
    );
}
