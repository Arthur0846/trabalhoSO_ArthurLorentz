        .text
_f_proc_init_privilegiado:
        ld      r0, 65
        outb    r0, (1)
        ret
