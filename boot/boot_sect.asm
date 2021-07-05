[org 0x600]

KERNEL_OFFSET equ 0x1200

DISK_LOAD_OFFSET equ 0x1000

;start by moving over the bootsector to address 0x600
;this way, the kernel (which will be loaded at 0x1000) will get alot of space
;first few lines are from https://wiki.osdev.org/MBR_(x86)
cli                         ; We do not want to be interrupted
xor ax, ax                  
mov ds, ax                  
mov es, ax                  
mov ss, ax                  
mov sp, ax                  
mov cx, 0x0100
mov si, 0x7C00              ; Current bootsector Address
mov di, 0x0600              ; New bootsector Address
rep movsw                   ; Move everything over
jmp 0:LowStart              ; Jump to the new location of the bootsector

LowStart:
sti
mov [BOOT_DRIVE], dl
mov bp, 0x5FF   ;setting up the stack
mov sp, bp

call switch_graphics   ;setup VGA graphics mode

call get_font  ;store a font table at 0x10000

call load_kernel  ;read the disk to load the kernel

call do_e820  ;ask BIOS for a memory map and place it at addres 0x11000

call enable_A20    ;make sure the A20 line is enabled

call switch_to_pm    ;switch to protected mode

jmp $

%include "boot/enable_A20.asm"
%include "boot/disk_load.asm"
%include "boot/gdt_table_memory.asm"
%include "boot/switch_to_pm.asm"
%include "boot/switch_graphics_mode.asm"
%include "boot/get_font.asm"
%include "boot/make_mem_table.asm"

[bits 16]

load_kernel:
    mov bx, DISK_LOAD_OFFSET
    mov dh, 80
    mov dl, [BOOT_DRIVE]
    call disk_load   ;read the disk
    
    ret

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET   ;execute kernel code
    
    jmp $
    
BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
