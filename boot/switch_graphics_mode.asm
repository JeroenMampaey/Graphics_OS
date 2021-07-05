[bits 16]
switch_graphics:
;this does assume that register ax is not yet used
	mov ah, 0x00
	mov al, 0x12
	int 0x10
	ret
