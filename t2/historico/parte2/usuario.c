#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
#define SYS_ESPERA_PROC 4

int so_chama(int num, int a1, int a2);

char *msg_init = "init: criando A, B e C\n";
char *msg_fim = "init: C terminou\n";

void escreve(char *s)
{
    while (*s != 0) {
        so_chama(SYS_ESCREVE, *s, 0);
        s = s + 1;
    }
}

void proc_a(void)
{
    escreve("A\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}

void proc_b(void)
{
    escreve("B\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}

void proc_c(void)
{
    int c;
    escreve("C? ");
    c = so_chama(SYS_LE, 0, 0);
    so_chama(SYS_ESCREVE, c, 0);
    escreve("\n");
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}

void init(void)
{
    int r;
    int pid_a;
    int pid_c;

    pid_a = so_chama(SYS_CRIA_PROC, (int) proc_a, 0);
    so_chama(SYS_CRIA_PROC, (int) proc_b, 0);
    pid_c = so_chama(SYS_CRIA_PROC, (int) proc_c, 0);
    escreve(msg_init);

    escreve("init: espera de pid inexistente -> ");
    r = so_chama(SYS_ESPERA_PROC, 999, 0);
    if (r == -1) {
        escreve("-1\n");
    } else {
        escreve("0\n");
    }

    escreve("init: auto-espera -> ");
    r = so_chama(SYS_ESPERA_PROC, 1, 0);
    if (r == -1) {
        escreve("-1\n");
    } else {
        escreve("0\n");
    }

    so_chama(SYS_ESPERA_PROC, pid_a, 0);
    escreve("init: A terminou\n");

    so_chama(SYS_ESPERA_PROC, pid_c, 0);
    escreve(msg_fim);
    so_chama(SYS_MATA_PROC, -1, 0);
    for (;;) {
    }
}
