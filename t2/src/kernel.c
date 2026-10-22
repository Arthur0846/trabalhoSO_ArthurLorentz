#include "config.h"

#define N_PROCESSOS 8
#define N_REGISTROS 256

#define MORTO  0
#define PRONTO 1
#define EXECUTANDO 2
#define BLOQUEADO 3

#define BLQ_NADA 0
#define BLQ_ES 1
#define BLQ_PROC 2

/* Apenas as 5 chamadas de sistema exigidas pelo enunciado. A politica de
   escalonamento, o quantum e o intervalo do relogio sao fixados em tempo de
   compilacao (config.h), nao por chamada de sistema. */
#define SYS_LE          0
#define SYS_ESCREVE     1
#define SYS_CRIA_PROC   2
#define SYS_MATA_PROC   3
#define SYS_ESPERA_PROC 4

#define ESCALA 256
#define PRIORIDADE_PADRAO (ESCALA / 2)

struct Processo {
    int quadro[16];
    int pid;
    int estado;
    int bloqueio;
    int espera_pid;
    int prioridade;
    int tempo_em[4];
    int entradas_em[4];
    int desde;
    int t_desbloq;
    int resp_total;
    int resp_n;
    int preemp;
    int t_criacao;
    int vida;
};

struct Processo processos[N_PROCESSOS];

struct Registro {
    int pid;
    int vida;
    int tempo_em[4];
    int entradas_em[4];
    int resp_total;
    int resp_n;
    int preemp;
};

struct Registro registros[N_REGISTROS];
int n_registros;
int registros_descartados;
int processo_atual;
int proximo_pid;
int so_contexto_mudou;
int so_sem_processos;
int fila[N_PROCESSOS];
int fila_n;
int quantum_restante;
int quantum_atual;
int escalonador_prioridade;
int ocioso_total;
int ocioso_inicio;
int preemp_total;
int int_relogio;
int int_syscalls;
int int_console;
int int_excecoes;
int intervalo_relogio;
int int_tipo[16];
int pid_init;

void dispositivo_escreve(int c);
int  dispositivo_tem_entrada(void);
int  dispositivo_le(void);
int  tempo(void);
int  le_tiques_bios(void);
void programa_relogio(int intervalo);
void so_conta_interrupcao(int vetor);
void desbloqueia_espera(int vitima);
void so_excecao(int *quadro, int vetor);
void so_relatorio(void);
void init(void);
void ocioso(void);

int pilhas[N_PROCESSOS][64];
int pilha_ociosa[64];

/* Tempo interno em ticks do relogio. O contador de ciclos do hardware nao e'
   confiavel entre chamadas do kernel: um processo CPU-bound pode faze-lo dar
   a volta varias vezes. A impressao converte ticks para instrucoes. */
int tempo(void)
{
    return le_tiques_bios();
}

void so_conta_interrupcao(int vetor)
{
    if (vetor >= 0 && vetor < 16) {
        int_tipo[vetor] = int_tipo[vetor] + 1;
    }
    if (vetor == 8) {
        int_console = int_console + 1;
    }
}

void muda_estado(int slot, int novo)
{
    int agora;
    int velho;

    agora = tempo();
    velho = processos[slot].estado;
    processos[slot].tempo_em[velho] = processos[slot].tempo_em[velho]
                                      + agora - processos[slot].desde;
    processos[slot].desde = agora;
    processos[slot].entradas_em[novo] = processos[slot].entradas_em[novo] + 1;
    processos[slot].estado = novo;
}

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

int fila_menor_prioridade(void)
{
    int i;
    int melhor;
    melhor = fila[0];
    for (i = 1; i < fila_n; i++) {
        if (processos[fila[i]].prioridade < processos[melhor].prioridade) {
            melhor = fila[i];
        }
    }
    return melhor;
}

void fila_insere(int slot)
{
    fila[fila_n] = slot;
    fila_n = fila_n + 1;
}

int fila_retira(void)
{
    int s;
    int i;
    s = fila[0];
    for (i = 1; i < fila_n; i++) {
        fila[i - 1] = fila[i];
    }
    fila_n = fila_n - 1;
    return s;
}

void fila_remove(int slot)
{
    int i;
    int j;
    for (i = 0; i < fila_n; i++) {
        if (fila[i] == slot) {
            for (j = i + 1; j < fila_n; j++) {
                fila[j - 1] = fila[j];
            }
            fila_n = fila_n - 1;
            return;
        }
    }
}

void monta_quadro_inicial(int slot, int entrada, int arg)
{
    int i;
    for (i = 0; i < 16; i++) {
        processos[slot].quadro[i] = 0;
    }
    pilhas[slot][61] = 0;
    pilhas[slot][62] = arg;
    processos[slot].quadro[6] = (int) &pilhas[slot][61];
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

int so_cria_proc(int entrada, int arg, int prioridade)
{
    int slot;
    int i;

    slot = slot_livre();
    if (slot < 0) {
        return -1;
    }
    monta_quadro_inicial(slot, entrada, arg);
    processos[slot].pid = proximo_pid;
    proximo_pid = proximo_pid + 1;
    processos[slot].bloqueio = BLQ_NADA;
    processos[slot].espera_pid = 0;
    processos[slot].prioridade = prioridade;
    processos[slot].preemp = 0;
    processos[slot].resp_total = 0;
    processos[slot].resp_n = 0;
    processos[slot].t_desbloq = -1;
    processos[slot].t_criacao = tempo();
    for (i = 0; i < 4; i++) {
        processos[slot].tempo_em[i] = 0;
        processos[slot].entradas_em[i] = 0;
    }
    processos[slot].desde = tempo();
    muda_estado(slot, PRONTO);
    fila_insere(slot);
    return processos[slot].pid;
}

void arquiva(int slot)
{
    if (n_registros >= N_REGISTROS) {
        registros_descartados = registros_descartados + 1;
        return;
    }
    registros[n_registros].pid = processos[slot].pid;
    registros[n_registros].vida = processos[slot].vida;
    registros[n_registros].resp_total = processos[slot].resp_total;
    registros[n_registros].resp_n = processos[slot].resp_n;
    registros[n_registros].preemp = processos[slot].preemp;
    registros[n_registros].tempo_em[MORTO] = processos[slot].tempo_em[MORTO];
    registros[n_registros].tempo_em[PRONTO] = processos[slot].tempo_em[PRONTO];
    registros[n_registros].tempo_em[EXECUTANDO] = processos[slot].tempo_em[EXECUTANDO];
    registros[n_registros].tempo_em[BLOQUEADO] = processos[slot].tempo_em[BLOQUEADO];
    registros[n_registros].entradas_em[MORTO] = processos[slot].entradas_em[MORTO];
    registros[n_registros].entradas_em[PRONTO] = processos[slot].entradas_em[PRONTO];
    registros[n_registros].entradas_em[EXECUTANDO] = processos[slot].entradas_em[EXECUTANDO];
    registros[n_registros].entradas_em[BLOQUEADO] = processos[slot].entradas_em[BLOQUEADO];
    n_registros = n_registros + 1;
}

void desbloqueia_espera(int vitima)
{
    int i;
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado == BLOQUEADO &&
            processos[i].bloqueio == BLQ_PROC &&
            processos[i].espera_pid == vitima) {
            processos[i].bloqueio = BLQ_NADA;
            muda_estado(i, PRONTO);
            processos[i].t_desbloq = tempo();
            fila_insere(i);
        }
    }
}

int so_mata_proc(int pid)
{
    int slot;
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
    if (processos[slot].estado == PRONTO) {
        fila_remove(slot);
    }
    muda_estado(slot, MORTO);
    processos[slot].vida = tempo() - processos[slot].t_criacao;
    processos[slot].bloqueio = BLQ_NADA;
    arquiva(slot);
    desbloqueia_espera(vitima);

    if (vitima == pid_init) {
        /* o enunciado pede o relatorio "quando o init morrer", entao dispara aqui. */
        so_relatorio();
        so_sem_processos = 1;
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
    muda_estado(processo_atual, BLOQUEADO);
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
    muda_estado(processo_atual, BLOQUEADO);
    processos[processo_atual].bloqueio = BLQ_ES;
    so_contexto_mudou = 1;
    return 0;
}

int so_escreve(int c)
{
    /* A console esta sempre pronta para saida (porta 0x02, bit 0); apenas
       SO_LE pode bloquear. */
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
            muda_estado(i, PRONTO);
            processos[i].t_desbloq = tempo();
            fila_insere(i);
        }
    }
}

void so_configura_escalonador(int modo, int novo_quantum, int novo_intervalo)
{
    escalonador_prioridade = modo;
    quantum_atual = novo_quantum;
    quantum_restante = novo_quantum;
    programa_relogio(novo_intervalo);
}

int so_despacha(int num, int a1, int a2, int a3, int a4)
{
    int_syscalls = int_syscalls + 1;
    int_tipo[7] = int_tipo[7] + 1;
    if (num == SYS_LE) {
        return so_le();
    }
    if (num == SYS_ESCREVE) {
        return so_escreve(a1);
    }
    if (num == SYS_CRIA_PROC) {
        return so_cria_proc(a1, a2, PRIORIDADE_PADRAO);
    }
    if (num == SYS_MATA_PROC) {
        return so_mata_proc(a1);
    }
    if (num == SYS_ESPERA_PROC) {
        return so_espera_proc(a1);
    }
    return -1;
}

void kernel_num(int n)
{
    int buf[6];
    int i;

    if (n < 0) {
        dispositivo_escreve('-');
        n = -n;
    }
    if (n == 0) {
        dispositivo_escreve('0');
        return;
    }
    i = 0;
    while (n > 0) {
        buf[i] = n % 10;
        n = n / 10;
        i = i + 1;
    }
    while (i > 0) {
        i = i - 1;
        dispositivo_escreve('0' + buf[i]);
    }
}

void kernel_texto(char *s)
{
    while (*s != 0) {
        dispositivo_escreve(*s);
        s = s + 1;
    }
}

/* Imprime n * multiplicador em decimal sem estourar o int de 16 bits.
   Digitos na base 100 (cada digito <= 99); produto parcial com um digito do multiplicador <= 99*655 < 32767 e carry pequeno, seguro. */
void kernel_mult(int n, int multiplicador)
{
    int a[8];
    int prod[10];
    int a_n;
    int p_n;
    int i;
    int j;
    int m;
    int carry;
    int t;

    a_n = 0;
    if (n == 0) {
        a[0] = 0;
        a_n = 1;
    }
    while (n > 0) {
        a[a_n] = n % 100;
        n = n / 100;
        a_n = a_n + 1;
    }
    if (multiplicador == 0) {
        a_n = 1;
        a[0] = 0;
    }
    p_n = 0;
    for (i = 0; i < 10; i++) {
        prod[i] = 0;
    }
    while (multiplicador > 0) {
        m = multiplicador % 100;
        carry = 0;
        for (i = 0; i < a_n; i++) {
            t = a[i] * m + prod[i + p_n] + carry;
            prod[i + p_n] = t % 100;
            carry = t / 100;
        }
        j = i + p_n;
        while (carry > 0) {
            prod[j] = prod[j] + carry;
            carry = prod[j] / 100;
            prod[j] = prod[j] % 100;
            j = j + 1;
        }
        multiplicador = multiplicador / 100;
        p_n = p_n + 1;
    }

    j = 9;
    while (j > 0 && prod[j] == 0) {
        j = j - 1;
    }
    for (i = j; i >= 0; i--) {
        if (i == j) {
            kernel_num(prod[i]);
        } else if (prod[i] < 10) {
            dispositivo_escreve('0');
            kernel_num(prod[i]);
        } else {
            kernel_num(prod[i]);
        }
    }
}

/* Imprime um tempo medido em ticks na unidade do enunciado: instrucoes (ticks * intervalo). */
void kernel_tempo(int ticks)
{
    kernel_mult(ticks, intervalo_relogio);
}

void so_relatorio(void)
{
    int i;

    kernel_texto("\n");
    kernel_texto("tempo = ticks x intervalo de relogio, em instrucoes\n");
    kernel_texto("intervalo do relogio = ");
    kernel_num(intervalo_relogio);
    kernel_texto(" instrucoes por tick\n");
    for (i = 0; i < n_registros; i++) {
        kernel_texto("p");
        kernel_num(registros[i].pid);
        kernel_texto(" v=");
        kernel_tempo(registros[i].vida);
        kernel_texto(" e=");
        kernel_tempo(registros[i].tempo_em[EXECUTANDO]);
        kernel_texto(" p=");
        kernel_tempo(registros[i].tempo_em[PRONTO]);
        kernel_texto(" b=");
        kernel_tempo(registros[i].tempo_em[BLOQUEADO]);
        kernel_texto(" pre=");
        kernel_num(registros[i].preemp);
        kernel_texto(" r=");
        if (registros[i].resp_n > 0) {
            kernel_tempo(registros[i].resp_total / registros[i].resp_n);
        } else {
            kernel_texto("-");
        }
        kernel_texto(" E=");
        kernel_num(registros[i].entradas_em[EXECUTANDO]);
        kernel_texto(" P=");
        kernel_num(registros[i].entradas_em[PRONTO]);
        kernel_texto(" B=");
        kernel_num(registros[i].entradas_em[BLOQUEADO]);
        kernel_texto("\n");
    }
    kernel_texto("SO t=");
    kernel_tempo(tempo());
    kernel_texto(" ocioso=");
    kernel_tempo(ocioso_total);
    kernel_texto(" irq r=");
    kernel_num(int_relogio);
    kernel_texto(" s=");
    kernel_num(int_syscalls);
    kernel_texto(" c=");
    kernel_num(int_console);
    kernel_texto(" x=");
    kernel_num(int_excecoes);
    kernel_texto(" pre=");
    kernel_num(preemp_total);
    kernel_texto(" proc=");
    kernel_num(proximo_pid - 1);
    kernel_texto("\n");
    if (registros_descartados > 0) {
        kernel_texto("AVISO: ");
        kernel_num(registros_descartados);
        kernel_texto(" registros descartados (tabela cheia)\n");
    }
    kernel_texto("irq por vetor:");
    for (i = 0; i < 16; i++) {
        if (int_tipo[i] != 0) {
            kernel_texto(" v");
            kernel_num(i);
            kernel_texto("=");
            kernel_num(int_tipo[i]);
        }
    }
    kernel_texto("\n");
}

void so_escalona(int *quadro)
{
    int interrompido;
    int escolhido;
    int t_exec;
    int i;

    interrompido = processo_atual;
    if (interrompido >= 0) {
        salva_quadro(interrompido, quadro);
        if (processos[interrompido].estado == EXECUTANDO) {
            /* prioridade: prio = (prio + t_exec/t_quantum)/2 em ponto fixo */
            t_exec = quantum_atual - quantum_restante;
            processos[interrompido].prioridade =
                (processos[interrompido].prioridade +
                 t_exec * ESCALA / quantum_atual) / 2;
            muda_estado(interrompido, PRONTO);
            fila_insere(interrompido);
            processos[interrompido].preemp = processos[interrompido].preemp + 1;
            preemp_total = preemp_total + 1;
        } else if (processos[interrompido].estado == BLOQUEADO) {
            t_exec = quantum_atual - quantum_restante;
            processos[interrompido].prioridade =
                (processos[interrompido].prioridade +
                 t_exec * ESCALA / quantum_atual) / 2;
        }
    }

    so_contexto_mudou = 0;

    escolhido = -1;
    if (fila_n > 0) {
        if (escalonador_prioridade) {
            escolhido = fila_menor_prioridade();
        } else {
            escolhido = fila[0];
        }
        fila_remove(escolhido);
    }

    if (escolhido >= 0) {
        if (escolhido != interrompido || quantum_restante <= 0) {
            quantum_restante = quantum_atual;
        }
        if (interrompido < 0) {
            ocioso_total = ocioso_total + tempo() - ocioso_inicio;
        }
        if (processos[escolhido].t_desbloq >= 0) {
            processos[escolhido].resp_total = processos[escolhido].resp_total
                                              + tempo() - processos[escolhido].t_desbloq;
            processos[escolhido].resp_n = processos[escolhido].resp_n + 1;
            processos[escolhido].t_desbloq = -1;
        }
        muda_estado(escolhido, EXECUTANDO);
        processo_atual = escolhido;
        restaura_quadro(escolhido, quadro);
        return;
    }

    processo_atual = -1;
    if (interrompido >= 0) {
        ocioso_inicio = tempo();
    }
    for (i = 0; i < N_PROCESSOS; i++) {
        if (processos[i].estado != MORTO) {
            monta_quadro_ocioso(quadro);
            return;
        }
    }
    if (!so_sem_processos) {
        so_relatorio();
    }
    so_sem_processos = 1;
}

void so_relogio(int *quadro)
{
    int_relogio = int_relogio + 1;
    int_tipo[10] = int_tipo[10] + 1;
    so_trata_pendencias();
    if (processo_atual >= 0) {
        quantum_restante = quantum_restante - 1;
        if (quantum_restante > 0) {
            return;
        }
    }
    so_escalona(quadro);
}

void so_excecao(int *quadro, int vetor)
{
    int culpado;
    int ip;

    int_excecoes = int_excecoes + 1;
    if (vetor >= 0 && vetor < 16) {
        int_tipo[vetor] = int_tipo[vetor] + 1;
    }
    /* s0 (sr) e a palavra 8 do quadro; falha em supervisor nao e do processo. */
    if (quadro[8] & 0x8000) {
        kernel_texto("\nPANICO: excecao no kernel v");
        kernel_num(vetor);
        kernel_texto(" ip=");
        kernel_num(quadro[7]);
        kernel_texto("\n");
        so_sem_processos = 1;
        return;
    }
    culpado = processo_atual;
    ip = quadro[7];
    kernel_texto("\nEXCECAO v");
    kernel_num(vetor);
    kernel_texto(" pid=");
    kernel_num(processos[culpado].pid);
    kernel_texto(" ip=");
    kernel_num(ip);
    kernel_texto("\n");
    salva_quadro(culpado, quadro);
    processos[culpado].bloqueio = BLQ_NADA;
    if (processos[culpado].estado == PRONTO) {
        fila_remove(culpado);
    }
    muda_estado(culpado, MORTO);
    processos[culpado].vida = tempo() - processos[culpado].t_criacao;
    arquiva(culpado);
    desbloqueia_espera(processos[culpado].pid);
    if (processos[culpado].pid == pid_init) {
        /* o enunciado pede o relatorio "quando o init morrer", tambem quando ele morre por excecao. */
        so_relatorio();
        so_sem_processos = 1;
    }
    so_contexto_mudou = 0;
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
    fila_n = 0;
    n_registros = 0;
    registros_descartados = 0;
    pid_init = 0;
    escalonador_prioridade = 0;
    quantum_atual = QUANTUM;
    quantum_restante = quantum_atual;
    ocioso_total = 0;
    ocioso_inicio = 0;
    preemp_total = 0;
    int_relogio = 0;
    int_syscalls = 0;
    int_console = 0;
    int_excecoes = 0;
    for (i = 0; i < 16; i++) {
        int_tipo[i] = 0;
    }

    /* Politica, quantum e intervalo vem de config.h - fonte unica para o relatorio e para o hardware. */
    so_configura_escalonador(POLITICA, QUANTUM, INTERVALO);

    so_cria_proc((int) init, 0, PRIORIDADE_PADRAO);
    pid_init = processos[0].pid;
    fila_retira();
    processo_atual = 0;
    muda_estado(0, EXECUTANDO);

    restaura_quadro(0, quadro_boot);
}