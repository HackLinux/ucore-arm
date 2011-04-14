#include <memmap.h>

struct physical_memmap mem_map;

void
init_physical_memmap() {
    mem_map.nr_map = 2;
    mem_map.map[0].addr = 0x00000000;
    mem_map.map[0].size = 0x30000000;
    mem_map.map[0].type = MEMMAP_RESERVED;
    mem_map.map[1].addr = 0x30000000;
    mem_map.map[1].size = 0x01800000;
    mem_map.map[1].type = MEMMAP_MEMORY;
}
