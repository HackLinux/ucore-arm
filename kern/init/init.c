#include <assert.h>
#include <clock.h>
#include <console.h>
#include <intr.h>
#include <stdio.h>
#include <string.h>
#include <atomic.h>

int kern_init(void) __attribute__((noreturn));

void test_atomic() {
    atomic_t a;
    atomic_set(&a, 10);
    cprintf("a: %d\n", atomic_read(&a));
    cprintf("a: %d\n", atomic_sub_return(&a, 9));
    cprintf("a=0: %d\n", atomic_dec_test_zero(&a));
    return;
}

int
kern_init(void) {
    // clear bss
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    // init console
    cons_init();

    // init interrupts
    intr_init();

    // init timer
    clock_init();

    // init interrupts
    intr_enable();

    // init message
    cprintf("\n(THU.CST) ucore(arm)\n");

    // test atomic
    test_atomic();

    // do nothing
    while(1);
}
