global fill_bytes_asm
global fill_dwords_asm

;fill_bytes_asm(int addres, int number_of_bytes)
;place one's for "number_of_bytes" bytes starting at address "address"
fill_bytes_asm:
    push ecx
    push edi
    push eax

    mov edi, dword [esp+16]
    mov al, 0xff

    mov ecx, [esp+20]
    rep stosb

    pop eax
    pop edi
    pop ecx
    ret

;fill_dwords_asm(int addres, int number_of_dwords)
;place one's for "number_of_dwords" dwords starting at address "address"
fill_dwords_asm:
    push ecx
    push edi
    push eax

    mov edi, dword [esp+16]
    mov eax, 0xffffffff

    mov ecx, [esp+20]
    rep stosd

    pop eax
    pop edi
    pop ecx
    ret