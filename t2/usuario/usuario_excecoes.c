#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
#define SYS_ESPERA_PROC 4
int so_chama(int num, int a1, int a2);
void proc_privilegiado(void);
void proc_divide_zero(void);
void escreve(char *s)
{
    while (*s != 0) {
        so_chama(SYS_ESCREVE, *s, 0);
        s = s + 1;
    }
}
void proc_ok(int n)
{
    int i;
    int x;
    x = 0;
    for (i = 0; i < n; i = i + 1) {
        x = x + i;
    }
    escreve("ok\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
void init(void)
{
    int p1;
    int p2;
    int p3;
    escreve("excecoes\n");
    p1 = so_chama(SYS_CRIA_PROC, (int) proc_privilegiado, 0);
    p2 = so_chama(SYS_CRIA_PROC, (int) proc_divide_zero, 0);
    p3 = so_chama(SYS_CRIA_PROC, (int) proc_ok, 2000);
    so_chama(SYS_ESPERA_PROC, p1, 0);
    so_chama(SYS_ESPERA_PROC, p2, 0);
    so_chama(SYS_ESPERA_PROC, p3, 0);
    escreve("fim\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
