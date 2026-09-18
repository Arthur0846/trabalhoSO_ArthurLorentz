        .equ    pilha = 0x3F0

        .org    0
        .dw     main, pilha, 0, 0

        .org    0x80
main:
        ldq     r0, 255
        out     r0, (0x22)
        out     r0, (0x23)
        in      r0, (0x20)
        shl     r0, 8
        in      r1, (0x21)
        or      r0, r1
        ld      r1, 25173
        mul     r0, r1
        add     r0, 13849
        call    imprime_hex
        halt

imprime_hex:
        ldq     r4, 4
prox_digito:
        ld      r3, r0
        shr     r3, 12
        cmp     r3, 10
        jmpc    lt, decimal
        addb    r3, 55
        bra     converte
decimal:
        addb    r3, 48
converte:
        push    r0
        ld      r0, r3
        call    putc
        pop     r0
        shl     r0, 4
        addq    r4, -1
        jmpc    nz, prox_digito
        ret

putc:
        inb     r2, (2)
        andb    r2, 1
        jmpc    z, putc
        outb    r0, (1)
        ret
