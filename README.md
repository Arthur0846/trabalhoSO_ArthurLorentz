# Trabalho de Sistemas Operacionais 2026/2

## T1 — Número aleatório em assembly Mancha

Programa em linguagem de montagem do processador Mancha que gera um número
aleatório de 16 bits e o imprime em 4 dígitos hexadecimais a cada execução
(ex.: `BF6D`).

### Arquivos

- `T1/aleatorio.asm` — código-fonte
- `T1/aleatorio.mob` — código objeto montado (é o que o simulador executa)

### Como executar

O montador e o simulador do Mancha ficam no projeto `simulador_completo/`,
que **não** faz parte deste repositório. Com ele em mãos, compile uma vez:

```sh
cd simulador_completo
make
```

Depois aponte o simulador para o `.mob` deste repositório (o programa já vem
montado). Troque `CAMINHO` pelo local onde você clonou este repositório:

```sh
./bin/simulador CAMINHO/T1/aleatorio.mob
```

Para regerar o `.mob` a partir do `.asm` (opcional):

```sh
./bin/montador CAMINHO/T1/aleatorio.asm -o CAMINHO/T1/aleatorio.mob
```

Requer terminal de pelo menos 90 colunas x 36 linhas. No simulador, `C`
executa e `F` sai.

### Como funciona

Lê o contador do relógio (portas `0x20`/`0x21`) como semente e aplica
`x = (x * 25173 + 13849) mod 65536`; imprime o resultado em hexadecimal.

## T2 — Sistema Operacional de Processos sobre o Mancha

Mini SO para o processador Mancha, com multiprogramação, cinco chamadas de
sistema, bloqueio de E/S, espera pela morte de processos, escalonamento
circular e por prioridade, preempção e métricas de execução. O código está em
`t2/`; os binários são gerados pelo `make`. As decisões de projeto, as
configurações testadas e os resultados estão em
[`t2/RELATORIO.md`](t2/RELATORIO.md).

### Dependências

O `Makefile` do T2 espera `bios_mancha/`, `compilador_c/` e
`simulador_completo/` na raiz deste repositório, ao lado de `t2/`. Essas pastas
fazem parte do projeto Mancha fornecido e não são versionadas aqui.

O build usa `gcc` para recompilar o `mcc` localmente (o binário original exige
GLIBC 2.38) e copia `montador` e `simulador` para `t2/build/tools/`. Nenhum
arquivo dos diretórios originais é alterado.

### Como compilar

```sh
cd t2
make
```

O comando gera os seis binários entregues:

- `parte1/t2p1.mob` — processos e multiprogramação;
- `parte2/t2p2.mob` — bloqueio de E/S e `SO_ESPERA_PROC`;
- `parte3/t2.mob` — escalonadores e métricas;
- `parte3/t2ex.mob` — exceções em processos filhos;
- `parte3/t2initex.mob` — exceção no processo init;
- `parte3/t2vivos.mob` — processos vivos no relatório final.

Os arquivos intermediários e os binários de experimento ficam em `t2/build/`,
que não faz parte da entrega.

### Como executar

A partir de `t2/`:

```sh
build/tools/simulador parte1/t2p1.mob
build/tools/simulador parte2/t2p2.mob
build/tools/simulador parte3/t2.mob
```

No simulador, use `D0` e depois `C` para executar; `F` sai. O terminal precisa
ter pelo menos 90 colunas por 36 linhas. Para a parte II, depois de `D0` e `C`,
digite `Eab` e Enter para enfileirar dois caracteres e desbloquear os dois
leitores.

Se precisar de mais linhas no console, `make simulador-largo` gera
`build/simulador_largo` com 60 linhas; nesse caso use
`TERM=xterm-256color COLUMNS=120 LINES=50` (necessário no WSL/Linux).

As saídas esperadas e os pontos que variam entre execuções estão em:

- `t2/parte1/SAIDA_ESPERADA.txt`
- `t2/parte2/SAIDA_ESPERADA.txt`
- `t2/parte3/SAIDA_ESPERADA.txt`

As evidências usadas no relatório ficam em `t2/parte1/resultados/`,
`t2/parte2/resultados/` e `t2/parte3/resultados/`.

Para gerar as configurações da parte III e os testes auxiliares:

```sh
make run-rr-q4
make run-prio-q4
make run-p1
make run-p2
make run-excecoes
make run-init-excecao
make run-vivos
```

Os binários de experimento ficam em `t2/build/exp/` e podem ser executados
com o mesmo simulador, por exemplo:

```sh
build/tools/simulador build/exp/out-P0-Q4-I4000-C2.mob
```

### Limpeza

```sh
cd t2
make clean       # remove apenas build/
make distclean   # remove build/ e os seis binários entregues
```

Não use o `make clean` da raiz do projeto Mancha, pois ele também apaga os
`.mob` dos exemplos originais.
