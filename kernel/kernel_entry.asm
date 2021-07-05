[bits 32]
[extern main]
[extern AP_main]
call main
jmp $
times 16-($-$$) db 0
call AP_main
jmp $