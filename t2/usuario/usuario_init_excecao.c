#include "config.h"

#define SYS_ESCREVE 1

int so_chama(int num, int a1, int a2);
void proc_init_privilegiado(void);

void init(void)
{
    so_chama(SYS_ESCREVE, 'I', 0);
    proc_init_privilegiado();
    so_chama(SYS_ESCREVE, 'X', 0);
    for (;;) {
    }
}
