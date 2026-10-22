        .text

        .equ    TICKS_POR_FATIA = 4000

_kernel_inicio:
        ld      r0, tabela_syscalls
        ld      r1, _trata_syscall
        st      r1, (r0)
        add     r0, 2
        st      r1, (r0)
        add     r0, 2
        st      r1, (r0)
        add     r0, 2
        st      r1, (r0)
        add     r0, 2
        st      r1, (r0)

        ld      r0, TICKS_POR_FATIA
        ld      r1, r0
        shr     r1, 8
        outb    r1, (0x22)
        outb    r0, (0x23)

        ld      r0, 5
        outb    r0, (0x30)

        ld      r0, escalonador_trampolim
        st      r0, (retomada_escalonador)

        ld      r0, pilha_sistema
        add     r0, -32
        push    r0
        call    _f_inicializa
        add     sp, 2

        ld      sp, pilha_sistema
        add     sp, -32
        rete

_trata_syscall:
        push    r4
        push    r3
        push    r2
        push    r1
        push    r0
        call    _f_so_despacha
        add     sp, 10

        ld      r1, (_g_so_contexto_mudou)
        cmp     r1, 0
        jmpc    eq, _so_fim_syscall

        ld      r1, pilha_sistema
        add     r1, -32
        st      r0, (r1)
        push    r1
        call    _f_so_escalona
        add     sp, 2

        ld      r1, (_g_so_sem_processos)
        cmp     r1, 0
        jmpc    ne, _so_parada

        ld      sp, pilha_sistema
        add     sp, -32
        rete

_so_fim_syscall:
        ret

_so_parada:
        halt

escalonador_trampolim:
        ld      r1, sp
        push    r1
        call    _f_so_relogio
        add     sp, 2

        ld      r1, (_g_so_sem_processos)
        cmp     r1, 0
        jmpc    ne, _so_parada

        ld      sp, pilha_sistema
        add     sp, -32
        rete

_f_so_chama:
        push    bp
        ld      bp, sp
        ld      r0, (bp+4)
        ld      r1, (bp+6)
        ld      r2, (bp+8)
        trap    7
        ld      sp, bp
        pop     bp
        ret

_f_dispositivo_escreve:
        push    bp
        ld      bp, sp
        ld      r0, (bp+4)
        outb    r0, (1)
        ld      sp, bp
        pop     bp
        ret

_f_dispositivo_tem_entrada:
        call    bios_console_disponivel
        ret

_f_dispositivo_le:
        call    bios_console_le
        ret
