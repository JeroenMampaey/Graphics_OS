C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c programs/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h programs/*.c)


#asm graphics functions in a different way?
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o drivers/screen_asm/clear.o drivers/screen_asm/move.o drivers/screen_asm/fill.o}

all: os_image.bin

os_image.bin: boot/boot_sect.bin boot/AP_boot.bin kernel.bin
	cat $^ > os_image.bin

kernel.bin: kernel/kernel_entry.o ${OBJ}
	ld -m elf_i386 -o $@ -T link.ld --oformat binary $^

%.o : %.c ${HEADERS}
	gcc -fno-pie -ffreestanding -m32 -c $< -o $@

%.o : %.asm
	nasm $< -f elf -o $@

%.bin : %.asm
	nasm -f bin -o $@ $<

clean:
	rm -fr *.bin *.dis *.o
	rm -fr kernel/*.o boot/*.bin drivers/*.o cpu/*.o libc/*.o drivers/screen_asm/*.o programs/*.o