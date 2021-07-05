[org 0x1000]
[bits 16]

AP_KERNEL_OFFSET equ 0x1210
    mov bx, 0x900
    lock inc byte [bx]

    jmp switch_to_pm

%include "boot/switch_to_pm_AP.asm"
%include "boot/gdt_table_memory.asm"

[bits 32]
BEGIN_PM:

    call AP_KERNEL_OFFSET
    
    jmp $

;make it 512 bytes big for alignment reasons (then I can put the BSP bootsector + AP bootsector + kernel code into one huge disk image)
times 512-($-$$) db 0