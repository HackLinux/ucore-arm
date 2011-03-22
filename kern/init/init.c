#include <console.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int kern_init(void) __attribute__((noreturn));

int
kern_init(void) {
    // clear bss
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    // init console
    cons_init();

    // init message
    cprintf("(THU.CST) ucore is loading...\n\n");

    panic("nothing to do!");

    // do nothing
    while(1);
}
