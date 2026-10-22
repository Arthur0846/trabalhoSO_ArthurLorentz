#define N_PROCESSOS 8

#define MORTO  0
#define PRONTO 1
#define EXECUTANDO 2
#define BLOQUEADO 3

#define BLQ_NADA 0
#define BLQ_ES   1
#define BLQ_PROC 2

#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
#define SYS_ESPERA_PROC 4

struct Processo {
    int quadro[16];
    int pid;
    int estado;
    int bloqueio;
    int espera_pid;
};

struct Processo processos[N_PROCESSOS];
int processo_atual;
int proximo_pid;
int so_contexto_mudou;
int so_sem_processos;

int  so_chama(int num, int a1, int a2);
void dispositivo_escreve(int c);
int  dispositivo_tem_entrada(void);
int  dispositivo_le(void);
void init(void);
void ocioso(void);

int pilhas[N_PROCESSOS][64];
int pilha_ociosa[64];

void salva_quadro(int slot, int *quadro)
{
    int i;
    for (i = 0; i < 16; i++) {
        processos[slot].quadro[i] = quadro[i];
    }
}

void restaura_quadro(int slot, int *quadro)
{
    int i;
    for (i = 0; i < 16; i++) {
        quadro[i] = processos[slot].quadro[i];
    }
}

int slot_do_pid(int pid)
{
    int i;
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado != MORTO && processos[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

int slot_livre(void)
{
    int i;
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado == MORTO) {
            return i;
        }
    }
    return -1;
}

void monta_quadro_inicial(int slot, int entrada)
{
    int i;
    for (i = 0; i < 16; i++) {
        processos[slot].quadro[i] = 0;
    }
    processos[slot].quadro[6] = (int) &pilhas[slot][64];
    processos[slot].quadro[7] = entrada;
}

void monta_quadro_ocioso(int *quadro)
{
    int i;
    for (i = 0; i < 16; i++) {
        quadro[i] = 0;
    }
    quadro[6] = (int) &pilha_ociosa[64];
    quadro[7] = (int) ocioso;
    quadro[8] = 0xA000;
}

void ocioso(void)
{
    for (;;) {
    }
}

int so_cria_proc(int entrada)
{
    int slot;
    slot = slot_livre();
    if (slot < 0) {
        return -1;
    }
    monta_quadro_inicial(slot, entrada);
    processos[slot].pid = proximo_pid;
    proximo_pid = proximo_pid + 1;
    processos[slot].estado = PRONTO;
    processos[slot].bloqueio = BLQ_NADA;
    processos[slot].espera_pid = 0;
    return processos[slot].pid;
}

int so_mata_proc(int pid)
{
    int slot;
    int i;
    int vitima;

    if (pid < 0) {
        slot = processo_atual;
    } else {
        slot = slot_do_pid(pid);
    }
    if (slot < 0) {
        return -1;
    }
    vitima = processos[slot].pid;
    processos[slot].estado = MORTO;
    processos[slot].bloqueio = BLQ_NADA;

    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado == BLOQUEADO &&
            processos[i].bloqueio == BLQ_PROC &&
            processos[i].espera_pid == vitima) {
            processos[i].estado = PRONTO;
            processos[i].bloqueio = BLQ_NADA;
        }
    }

    if (slot == processo_atual) {
        so_contexto_mudou = 1;
    }
    return 0;
}

int so_espera_proc(int pid)
{
    int slot;

    if (pid < 0) {
        return -1;
    }
    slot = slot_do_pid(pid);
    if (slot < 0) {
        return -1;
    }
    if (slot == processo_atual) {
        return -1;
    }
    processos[processo_atual].estado = BLOQUEADO;
    processos[processo_atual].bloqueio = BLQ_PROC;
    processos[processo_atual].espera_pid = pid;
    so_contexto_mudou = 1;
    return 0;
}

int so_le(void)
{
    if (dispositivo_tem_entrada()) {
        return dispositivo_le();
    }
    processos[processo_atual].estado = BLOQUEADO;
    processos[processo_atual].bloqueio = BLQ_ES;
    so_contexto_mudou = 1;
    return 0;
}

int so_escreve(int c)
{
    dispositivo_escreve(c & 255);
    return 0;
}

void so_trata_pendencias(void)
{
    int i;
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado == BLOQUEADO &&
            processos[i].bloqueio == BLQ_ES &&
            dispositivo_tem_entrada()) {
            processos[i].quadro[0] = dispositivo_le();
            processos[i].bloqueio = BLQ_NADA;
            processos[i].estado = PRONTO;
        }
    }
}

int so_despacha(int num, int a1, int a2, int a3, int a4)
{
    if (num == SYS_LE) {
        return so_le();
    }
    if (num == SYS_ESCREVE) {
        return so_escreve(a1);
    }
    if (num == SYS_CRIA_PROC) {
        return so_cria_proc(a1);
    }
    if (num == SYS_MATA_PROC) {
        return so_mata_proc(a1);
    }
    if (num == SYS_ESPERA_PROC) {
        return so_espera_proc(a1);
    }
    return -1;
}

void so_escalona(int *quadro)
{
    int atual;
    int escolhido;
    int i;
    int vivo;

    atual = processo_atual;
    if (atual >= 0) {
        salva_quadro(atual, quadro);
        if (processos[atual].estado == EXECUTANDO) {
            processos[atual].estado = PRONTO;
        }
    }

    so_trata_pendencias();

    escolhido = -1;
    if (atual >= 0 && processos[atual].estado == PRONTO) {
        escolhido = atual;
    } else {
        for (i = 0; i < N_PROCESSOS; i++) {
            if (processos[i].estado == PRONTO) {
                escolhido = i;
                break;
            }
        }
    }

    so_contexto_mudou = 0;

    if (escolhido >= 0) {
        processo_atual = escolhido;
        processos[escolhido].estado = EXECUTANDO;
        restaura_quadro(escolhido, quadro);
        return;
    }

    processo_atual = -1;
    vivo = 0;
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado != MORTO) {
            vivo = 1;
        }
    }
    if (vivo) {
        monta_quadro_ocioso(quadro);
        return;
    }
    so_sem_processos = 1;
}

void so_relogio(int *quadro)
{
    so_escalona(quadro);
}

void inicializa(int *quadro_boot)
{
    int i;

    for (i = 0; i < N_PROCESSOS; i++) {
        processos[i].estado = MORTO;
        processos[i].pid = 0;
        processos[i].bloqueio = BLQ_NADA;
        processos[i].espera_pid = 0;
    }
    processo_atual = -1;
    proximo_pid = 1;
    so_contexto_mudou = 0;
    so_sem_processos = 0;

    so_cria_proc((int) init);
    processo_atual = 0;
    processos[0].estado = EXECUTANDO;

    restaura_quadro(0, quadro_boot);
}
