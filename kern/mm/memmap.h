#ifndef __KERN_MM_MEMMAP_H__
#define __KERN_MM_MEMMAP_H__

#include <types.h>
#include <arch_memmap.h>

/* Implementations(memmap.c) goes into arch directory */

#define MEMMAP_ENTRY_MAX 20
#define MEMMAP_MEMORY       1       // address range memory
#define MEMMAP_RESERVED     2       // address range reserved

struct physical_memmap_entry {
    uint64_t addr;
    uint64_t size;
    uint32_t type;
} __attribute__((packed));

struct physical_memmap {
    int nr_map;
    struct physical_memmap_entry map[MEMMAP_ENTRY_MAX];
};

extern struct physical_memmap mem_map;

void init_physical_memmap();

#endif /* !__KERN_MM_MEMMAP_H__ */
