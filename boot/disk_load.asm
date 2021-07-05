disk_load:
    push dx
    
    mov ah, 0x02    ;read sector function
    mov al, dh      ;read DH sectors
    mov ch, 0x00    ;cilinder 0
    mov dh, 0x00    ;head 0
    mov cl, 0x02    ;start from sector 2 (sector after the boot sector)
    
    int 0x13
    
    jc disk_error   ;error will be made visible by the carry flag
    
    pop dx
    cmp dh, al
    jne disk_error  ;al is equal to the amount of sectors read
    ret
    
disk_error:
    jmp $
