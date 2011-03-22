#include <types.h>
#include <elf.h>

#define UFCON0  ((volatile unsigned int *)(0x50000020))

// We need to load this elf file.
#define INTIAL_LOAD_ADDRESS 0x30000000
#define ELFHDR ((struct elfhdr *)(INTIAL_LOAD_ADDRESS + 0x10000))

static const char *loading="ucore bootloader\n";
static const char *calling="Calling into kernel...\n";
static const char *bad="Bad Kernel, cannot be loaded.\n";
static const char *finished="No!!!You cannot come here!!!\n";

static void
uart_putc(int c) {
    *UFCON0 = c;
}

static void
uart_puts(const char* p) {
    while(*p)
        uart_putc(*p++);
}

void *
memcpy(void *dst, const void *src, size_t n) {
    const char *s = src;
    char *d = dst;
    while (n -- > 0) {
        *d ++ = *s ++;
    }
    return dst;
}

/* *
 * readseg - read @count bytes at @offset from kernel into virtual address @va.
 * currently we don't have any storage access method, assuming the kernel file
 * is already loaded at the correct location.
 * */
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    memcpy((void*)va, (void*)((uint8_t*)ELFHDR + offset), count);
}

void
bootmain(void){
    uart_puts(loading);

    // is this a valid ELF?
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }

    struct proghdr *ph, *eph;

    // load each program segment (ignores ph flags)
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;
    for (; ph < eph; ph ++) {
        readseg(ph->p_va, ph->p_memsz, ph->p_offset);
    }

    // call the entry point from the ELF header
    // note: does not return
    uart_puts(calling);
    ((void (*)(void))(ELFHDR->e_entry))();

    uart_puts(finished);
    while(1);
bad:
    uart_puts(bad);
    while(1);
}
