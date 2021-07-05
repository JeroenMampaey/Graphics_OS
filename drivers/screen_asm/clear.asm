global clear_dwords_asm

;clear_screen_asm(int addres, int number_of_dwords)
;place zeros for "number_of_dwords" dword starting at address "address"
clear_dwords_asm:
    push ecx
    push edi
    push eax

    mov edi, dword [esp+16]
    mov eax, 0

    mov ecx, [esp+20]
    rep stosd

    pop eax
    pop edi
    pop ecx
    ret