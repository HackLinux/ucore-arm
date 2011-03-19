#include <kern/driver/console.h>
#include <lib/stdio.h>

void
kern_init(void) {
    cons_init();
    cprintf("Hello World!\n");
    while(1);
}
