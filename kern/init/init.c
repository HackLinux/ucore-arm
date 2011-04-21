#include <assert.h>
#include <clock.h>
#include <console.h>
#include <intr.h>
#include <stdio.h>
#include <string.h>
#include <atomic.h>
#include <pmm.h>

int kern_init(void) __attribute__((noreturn));

int
kern_init(void) {
    // clear bss
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    // init console
    cons_init();

    cprintf("Hello World!.\n"); 

    // init pmm
    pmm_init();

    // init interrupts
    intr_init();

    // init timer
    clock_init();

    // init interrupts
    intr_enable();

    // init message
    cprintf("\n(THU.CST) ucore(arm)\n");

    // do nothing
    while(1);
}
