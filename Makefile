SKYEYE 		:= skyeye

TARGET_PREFIX	:= arm-elf-

HOSTCC		:= gcc
HOSTCFLAGS	:= -g -Wall -O2

CC			:= $(TARGET_PREFIX)gcc
CFLAGS		:= -fno-builtin -Wall -ggdb -nostdinc -nostdlib
CTYPE		:= c S

LD			:= $(TARGET_PREFIX)ld
LDFALGS		:= 

OBJCOPY		:= $(TARGET_PREFIX)objcopy
OBJDUMP		:= $(TARGET_PREFIX)objdump

COPY		:= cp
MKDIR		:= mkdir -p
MV			:= mv
RM			:= rm -f

bin/boot.bin: obj/boot/boot.o
	mkdir -p bin
	$(OBJCOPY) -O binary $< $@

obj/boot/boot.o: boot/bootmain.c boot/init.s
	mkdir -p obj/boot
	$(CC)	$(CFLAGS) -c -oobj/boot/bootmain.o boot/bootmain.c
	$(CC)	$(CFLAGS) -c -oobj/boot/init.o boot/init.s
	$(CC) 	$(CFLAGS) $(LDFALGS) -Tboot/boot.ld -o $@ obj/boot/bootmain.o obj/boot/init.o

skyeye:
	cd emu && skyeye

.PHONY: clean
clean:
	$(RM) -r obj
	$(RM) -r bin
