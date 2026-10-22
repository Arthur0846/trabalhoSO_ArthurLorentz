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
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void proc_longo(void)
{
    int i;
    for (i = 0; i < 20000; i = i + 1) {
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
int pids[12];
void teste_pid_monotonico(void)
{
    int p;
    int i;
    escreve("A pids:");
    for (i = 0; i < 3; i++) {
        p = so_chama(SYS_CRIA_PROC, (int) proc_curto, 0);
        escreve(" ");
        escreve_num(p);
        so_chama(SYS_ESPERA_PROC, p, 0);
    }
    escreve("\n");
}
void teste_tabela_cheia(void)
{
    int p;
    int i;
    int n;
    escreve("B tabela:");
    n = 0;
    for (i = 0; i < 12; i++) {
        p = so_chama(SYS_CRIA_PROC, (int) proc_longo, 0);
        if (p < 0) {
            escreve(" cheio(-1)");
            break;
        }
        pids[n] = p;
        n = n + 1;
    }
    escreve(" n=");
    escreve_num(n);
    escreve("\n");
    for (i = 0; i < n; i++) {
        so_chama(SYS_MATA_PROC, pids[i], 0);
    }
}
void teste_esperas_erradas(void)
{
    int r;
    escreve("C erros:");
    r = so_chama(SYS_ESPERA_PROC, 9999, 0);
    escreve(" inexist=");
    escreve_num(r);
    escreve(", auto=");
    r = so_chama(SYS_ESPERA_PROC, 1, 0);
    escreve_num(r);
    escreve("\n");
}
void init(void)
{
    int i;
    int x;
    escreve("ini p1\n");
    teste_pid_monotonico();
    teste_tabela_cheia();
    teste_esperas_erradas();
    escreve("fim\n");
    x = 0;
    for (i = 0; i < 4000; i = i + 1) {
        x = x + i;
    }
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
