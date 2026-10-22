#include "config.h"
#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
#define SYS_ESPERA_PROC 4
int so_chama(int num, int a1, int a2);
void escreve(char *s)
{
    while (*s != 0) {
        so_chama(SYS_ESCREVE, *s, 0);
        s = s + 1;
    }
}
void escreve_num(int v)
{
    int buf[6];
    int i;
    if (v < 0) {
        so_chama(SYS_ESCREVE, '-', 0);
        v = -v;
    }
    if (v == 0) {
        so_chama(SYS_ESCREVE, '0', 0);
        return;
    }
    i = 0;
    while (v > 0) {
        buf[i] = '0' + v % 10;
        v = v / 10;
        i = i + 1;
    }
    while (i > 0) {
        i = i - 1;
        so_chama(SYS_ESCREVE, buf[i], 0);
    }
}
void proc_curto(void)
{
    int i;
    int x;
    x = 0;
    for (i = 0; i < 300; i = i + 1) {
        x = x + i;
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_le(void)
{
    int c;
    escreve("le:");
    c = so_chama(SYS_LE, 0, 0);
    escreve(" [");
    so_chama(SYS_ESCREVE, c, 0);
    escreve("]\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void teste_espera_vivo(void)
{
    int p;
    int r;
    p = so_chama(SYS_CRIA_PROC, (int) proc_curto, 0);
    escreve("espera vivo=");
    r = so_chama(SYS_ESPERA_PROC, p, 0);
    escreve_num(r);
    escreve("\n");
}
void init(void)
{
    int a;
    int b;
    escreve("ini p2\n");
    teste_espera_vivo();
    a = so_chama(SYS_CRIA_PROC, (int) proc_le, 0);
    b = so_chama(SYS_CRIA_PROC, (int) proc_le, 0);
    so_chama(SYS_ESPERA_PROC, a, 0);
    so_chama(SYS_ESPERA_PROC, b, 0);
    escreve("fim\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
