#include <kern/driver/console.h>
#include <lib/stdio.h>

void
kern_init(void) {
    cons_init();
    cprintf("%d\n", 12345678);
    cprintf("Hello World!\n");
    while(1);
}
