        .text
_f_proc_privilegiado:
        outb    r0, (1)
_priv_laco:
        jmp     _priv_laco
_f_proc_divide_zero:
        ld      r0, 10
        ld      r1, 0
        div     r0, r1
_zer_laco:
        jmp     _zer_laco
