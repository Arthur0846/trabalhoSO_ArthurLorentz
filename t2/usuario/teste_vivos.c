#include "config.h"
#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
int so_chama(int num, int a1, int a2);
void filho_laco(void)
{
    for (;;) {
    }
}
void filho_le(void)
{
    so_chama(SYS_LE, 0, 0);
    for (;;) {
    }
}
void init(void)
{
    int i;
    so_chama(SYS_CRIA_PROC, (int) filho_laco, 0);
    so_chama(SYS_CRIA_PROC, (int) filho_le, 0);
    for (i = 0; i < 6000; i++) {
    }
    so_chama(SYS_ESCREVE, 'f', 0);
    so_chama(SYS_MATA_PROC, 0 - 1, 0);
}
