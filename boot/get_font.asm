[bits 16]
;reading font table from the VGA BIOS and storing it at 0x10000
;routine found at https://wiki.osdev.org/VGA_Fonts
;assumes registers ax, bx, cx are not yet used
get_font:
    push es
    push di
    ;font table will be stored at es:di <-> es*16+di <-> 0x10000
    mov ax, 0x1000
    mov es, ax
    mov ax, 0x0000
    mov di, ax

    push ds
    push es
    ;es:bp will contain the addres of the font table
    mov ax, 0x1130
    mov bh, 6
    int 0x10
    ;make sure ds:si contains the addres of the font table
    push es
    pop	ds
    pop	es
    mov	si, bp
    ;move over all 256*16 bytes   <-> 256 characters of 16*8 bits
    mov	cx, 256*16/4
    rep	movsd
    pop	ds

    pop di
    pop es
    ret
