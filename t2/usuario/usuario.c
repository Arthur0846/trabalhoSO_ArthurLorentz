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
void proc_filho(void)
{
    so_chama(SYS_ESCREVE, '.', 0);
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_puro(int n)
{
    int i;
    int x;
    x = 0;
    for (i = 0; i < n; i = i + 1) {
        x = x + i;
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_bloq(int n)
{
    int i;
    int filho;
    for (i = 0; i < n; i = i + 1) {
        filho = so_chama(SYS_CRIA_PROC, (int) proc_filho, 0);
        so_chama(SYS_ESPERA_PROC, filho, 0);
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_misto(int n)
{
    int i;
    int x;
    int filho;
    x = 0;
    for (i = 0; i < n; i = i + 1) {
        x = x + i;
        if (i % 512 == 0) {
            filho = so_chama(SYS_CRIA_PROC, (int) proc_filho, 0);
            so_chama(SYS_ESPERA_PROC, filho, 0);
        }
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_le(void)
{
    int c;
    so_chama(SYS_ESCREVE, '>', 0);
    c = so_chama(SYS_LE, 0, 0);
    so_chama(SYS_ESCREVE, c, 0);
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void init(void)
{
    int p;
    int i;
    int ps[4];
    escreve("ini\n");
#if CARGA == 0
    p = so_chama(SYS_CRIA_PROC, (int) proc_puro, 3000);
    so_chama(SYS_ESPERA_PROC, p, 0);
#endif
#if CARGA == 1
    p = so_chama(SYS_CRIA_PROC, (int) proc_bloq, 5);
    so_chama(SYS_ESPERA_PROC, p, 0);
#endif
#if CARGA == 2
    p = so_chama(SYS_CRIA_PROC, (int) proc_misto, 3000);
    so_chama(SYS_ESPERA_PROC, p, 0);
#endif
#if CARGA == 3
    p = so_chama(SYS_CRIA_PROC, (int) proc_le, 0);
    so_chama(SYS_ESPERA_PROC, p, 0);
#endif
#if CARGA == 4
    for (i = 0; i < 4; i = i + 1) {
        ps[i] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 3000);
    }
    for (i = 0; i < 4; i = i + 1) {
        so_chama(SYS_ESPERA_PROC, ps[i], 0);
    }
#endif
#if CARGA == 5
    ps[0] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 500);
    ps[1] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 1500);
    ps[2] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 3000);
    ps[3] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 6000);
    for (i = 0; i < 4; i = i + 1) {
        so_chama(SYS_ESPERA_PROC, ps[i], 0);
    }
#endif
#if CARGA == 6
    ps[0] = so_chama(SYS_CRIA_PROC, (int) proc_puro, 8000);
    ps[1] = so_chama(SYS_CRIA_PROC, (int) proc_bloq, 8);
    ps[2] = so_chama(SYS_CRIA_PROC, (int) proc_misto, 4000);
    for (i = 0; i < 3; i = i + 1) {
        so_chama(SYS_ESPERA_PROC, ps[i], 0);
    }
#endif
    escreve("fim\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
