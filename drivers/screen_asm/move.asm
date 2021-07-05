global move_dwords_asm

;move_asm(int from, int to, int number_of_dwords)
;move "number_of_dwords" dword from address "from" to address "to"
move_dwords_asm:
    push ecx
    push edi
    push esi

    mov esi, [esp+16]
    mov edi, [esp+20]

    mov	ecx, [esp+24]
    rep	movsd

    pop esi
    pop edi
    pop ecx
    ret