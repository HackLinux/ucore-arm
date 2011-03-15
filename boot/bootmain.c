#define UFCON0  ((volatile unsigned int *)(0x50000020))

static void
uart_putc(int c) {
    *UFCON0 = c;
}

void
bootmain(void){
    const char *p="Hello World!\n";
    while(*p){
        uart_putc(*p++);
    }
    while(1);
}
