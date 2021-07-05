shift_right:
    push bx
    push ax
while_loopke:
    mov bx, ax
    and bx, 0x0001
    cmp bx, 0x0001
    je end_while
    shr ax, 4
    shr cx, 4
    jmp while_loopke
end_while:
    pop ax
    pop bx
    ret

print_hex:
    push ax
    push bx
    push cx
    mov bx, HEX_OUT
    add bx, 0x0005
    mov ax, 0x000f
for_loop:
    cmp ax, 0x0000
    je end_for
    mov cx, dx
    and cx, ax
    call shift_right
    cmp cx, 0x000a
    jl cx_is_smaller_than_ten
    add cx, 39
cx_is_smaller_than_ten:
    add cx, '0'
    mov byte [bx], cl
    sub bx, 0x0001
    shl ax, 4
    jmp for_loop
end_for:
    pop cx
    pop bx
    pop ax
    mov bx, HEX_OUT
    call print_string
    ret

HEX_OUT:
    db '0x0000', 0
