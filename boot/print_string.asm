print_string:
push bx
push ax
mov ah, 0x0e
while_loop:
cmp byte [bx], 0
je end
mov al, [bx]
int 0x10
add bx, 0x01
jmp while_loop
end:
pop ax
pop bx
ret
