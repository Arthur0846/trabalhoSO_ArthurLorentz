# T2 -- Sistema Operacional de Processos sobre o Mancha

Relatório de execuções. Todo o código do trabalho está em `t2/`; nenhum
fonte original do Mancha (`bios_mancha/`, `compilador_c/`,
`simulador_completo/`) foi alterado, e os três binários originais foram
restaurados byte a byte com o modo `666` do zip. Para trabalhar nesta máquina,
o `Makefile` cria ferramentas locais descartáveis em
`build/tools/`: o `mcc` é recompilado dos fontes originais e o `montador` e o
`simulador` são copiados dos binários originais, recebendo permissão de
execução apenas nas cópias. O `mcc` do pacote exige `GLIBC_2.38` e esta
máquina tem `GLIBC_2.35`; por isso ele é recompilado localmente. Nada em
`compilador_c/bin/` ou `simulador_completo/bin/` é escrito. Instruções exatas
na seção 6.

**Organização de `t2/`.** O kernel é único e incremental (as partes I--III
constroem o mesmo `kernel.c`), então as fontes do SO existem uma única vez, em
`src/`; o que muda por parte é o programa de usuário (em `usuario/`) e os
binários e evidências, que ficam em `parte1/`, `parte2/` e `parte3/`:

```
t2/
  Makefile            tudo se faz daqui (make, make run-*, make clean, make distclean)
  RELATORIO.md        este arquivo
  src/                o SO: kernel.c (C) e ponte.asm (assembly)
  usuario/            programas de usuario: cargas e testes de cada parte
  parte1/ parte2/ parte3/
    *.mob             binario entregue e variantes de teste
    SAIDA_ESPERADA.txt  o que o binario deve imprimir
    resultados/       evidencias (saidas das execucoes)
  build/              GERADO e descartavel (config.h, .asm, experimentos, tools/); nao entregue
  historico/parte2/   fontes ao final da parte II; nao participam do build
```

| Parte                             | Programa de usuário                                                                                                     | Binário                                                        | Evidência                                                                   |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| I -- processos/multiprogramação | `usuario/teste_parte1.c`                                                                                               | `parte1/t2p1.mob`                                             | `parte1/resultados/t2p1.txt`                                               |
| II -- bloqueio e E/S              | `usuario/teste_parte2.c`                                                                                               | `parte2/t2p2.mob`                                             | `parte2/resultados/t2p2.txt`                                               |
| III -- escalonadores/exceções   | `usuario/usuario.c`, `usuario/usuario_excecoes.c`, `usuario/usuario_init_excecao.c` e rotinas em `usuario/*.asm` | `parte3/t2.mob`, `parte3/t2ex.mob`, `parte3/t2initex.mob` | `parte3/resultados/t2.txt`, `t2ex.txt`, `t2initex.txt` e `out-*.txt` |

Em cada `parteN/` há um `SAIDA_ESPERADA.txt` com as saídas determinísticas
e os pontos variáveis de cada binário, para conferir as execuções.

Os binários de experimento da parte III (`out-*.mob`) **não são entregues**:
são regeneráveis com `make run-*` e ficam em `build/exp/` (o `make clean`
apaga `build/` inteiro). O que se guarda como evidência é a saída `.txt` de
cada execução, em `parte3/resultados/`.

`historico/parte2/` guarda as fontes (kernel, ponte, usuário e Makefile) como
estavam ao final da parte II, para comparação. Esse diretório não participa do
build nem da entrega; a versão final é `src/` + `usuario/`.

## 1. Descrição do projeto

Foi implementado um mini SO para o processador Mancha, dividido nas tres
partes do enunciado:

- **Parte I -- processos e multiprogramação:** tipo `struct Processo`, tabela
  `processos[N_PROCESSOS]`, variável `processo_atual` (com suporte a nenhum
  processo, valor `-1`), `salva_quadro`/`restaura_quadro` (quadro de 16
  palavras), `so_escalona`, criação (`SO_CRIA_PROC`) e morte
  (`SO_MATA_PROC`) de processos. O `pid` é um número de série monótono
  (`proximo_pid`); a entrada na tabela pode ser reaproveitada por outro
  processo, o `pid` não.
- **Parte II -- bloqueio e E/S sem espera ocupada:** `SO_LE` bloqueia o
  processo se a console não tem entrada; `so_trata_pendencias` (chamada a
  cada interrupção do relógio) desbloqueia e entrega o caractere em
  `r0`. `SO_ESPERA_PROC` bloqueia o chamador ate o processo alvo morrer,
  retornando erro (`-1`) para pid inexistente ou para o próprio pid.
- **Parte III -- escalonadores preemptivos:** circular (fila FIFO, quantum
  em interrupcoes do relógio, processo preemptado vai ao fim da fila) e por
  prioridade (`prio = (prio + t_exec/t_quantum)/2` em ponto fixo, prioridade
  inicial recebida como parâmetro por `so_cria_proc`; `SO_CRIA_PROC` e o
  init passam o default 0,5, menor valor = maior prioridade). Métricas no fim
  da execução.

A ponte `ponte.asm` cuida do que o C do `mcc` não expõe: acesso a porta
(`outb` para programar o relógio e o controlador de interrupções, `in` para ler o contador 0x20), a
instalação dos vetores de exceção e de console, os trampolins
exceção/console/relógio e o primeiro `rete`. O restante (tabela de
processos, escalonador, chamadas de sistema, relatório) é C.

**Correção da medição de tempo (resolução 1 instrução):** adicionada leitura do
contador do relógio (porta 0x20) via `_f_le_contador_relogio` na ponte, e
função `tempo_instrucoes()` no kernel que computa `ticks * intervalo + contador`
em 32 bits (base 100). A impressão usa `kernel_tempo32(hi, lo)`. Com isso a
resolução passa de 1 tick (4000 instr) para 1 instrução; `SO t` agora bate com
o tempo real do simulador (erro ~1.700 instr = boot antes do relógio).

Não há `ei` na ponte, e não é preciso: o quadro de cada processo é montado
zerado, de modo que o `sr` do usuário nasce com `S=0`, `I=0` e `D=0` — as
interrupções externas já estão habilitadas quando o processo entra em
execução, e é por isso que um `outb` feito pelo processo gera a exceção 2
(instrução privilegiada) em vez de escrever na porta. Dentro do kernel o
bit `D` fica em 1 (a CPU o liga sozinha a cada interrupção) e é restaurado
pelo `rete` a partir do quadro do processo, então o SO nunca precisa
religar interrupções à mão.

### Decisões de projeto

- **Apenas as 5 chamadas de sistema** do enunciado (`SO_LE`, `SO_ESCREVE`, `SO_CRIA_PROC`, `SO_MATA_PROC`, `SO_ESPERA_PROC`). A política de escalonamento, o quantum e o intervalo do relógio são fixados em tempo de compilação em `build/config.h`. O `Makefile` gera o `config.h` e recompila `src/kernel.c` e `usuario/usuario.c`, de modo que **uma execução = uma configuração**.
- **Intervalo do relógio como fonte única:** o kernel programa o relógio com
  `programa_relogio(INTERVALO)` dentro de `inicializa()`, usando o mesmo
  `INTERVALO` que o relatório declara. Não há valor duplicado no `.asm`.
- **Unidade de tempo:** o enunciado pede tempo em instruções "medidas pelo
  relógio". Internamente o SO agora usa **tempo de 32 bits**: `ticks * intervalo + contador_do_relogio` (porta 0x20), obtendo **resolução de 1 instrução**. A multiplicação `ticks * intervalo` e a soma do contador são feitas em base 100 (função `tempo_instrucoes`) para não estourar o `int` de 16 bits. No relatório, a impressão usa `kernel_tempo32(hi, lo)`.
- **SO_ESCREVE nunca bloqueia:** a console do Mancha está sempre pronta para
  saída (bit0 do registrador de estado, porta `0x02`, é sempre 1 e a escrita
  nunca falha). Somente `SO_LE` bloqueia.
- **Pilha do kernel × `pilha_sistema`:** todo quadro do vetor da BIOS tem
  `sp` fixo (`pilha_sistema`, 0xF000) e a CPU reescreve `sp` para lá a cada
  interrupção ou `trap` — nada que precise sobreviver a um `rete` pode morar
  nessa área. Por isso o kernel roda em `pilha_nucleo` (0xE000), onde o
  `_start` da BIOS já o deixa antes de qualquer `call`, e o contexto de cada
  processo fica na tabela (`processos[slot].quadro`), nunca na pilha. Durante
  um atendimento, o trampolim usa só a região entre `pilha_nucleo` e
  `pilha_sistema`; o `rete` final lê o quadro do processo escolhido de volta
  no endereço fixo, escrito por `restaura_quadro`. As pilhas dos processos
  são um vetor global (`pilhas[]`), uma por entrada da tabela, então
  processos diferentes nunca compartilham memória de pilha.
- **Quadro do processo = 16 palavras zeradas, menos `sp` e `ip`:** o `sr`
  do usuário fica `0` (`S=0`, `I=0`, `D=0`), como o enunciado pede, e o
  `sp`/`ip` apontam para a pilha e o ponto de entrada do processo. O
  endereço de retorno deixado na pilha é `0`: um programa de usuário que
  retornasse saltaria para o vetor de interrupções (que não é código
  executável) e morreria por exceção; os programas entregues terminam em
  laço infinito, como nos exemplos da BIOS, então esse endereço nunca é
  usado.
- **Contagem de preempções:** `pre` conta cada quantum expirado, conforme a
  definição operacional do enunciado (o processo volta ao fim da fila),
  mesmo quando ele é o único processo pronto e acaba selecionado de novo.
  `E` conta quantas vezes o processo entrou em execução; as duas métricas
  não são intercambiáveis.
- **Biblioteca C do mcc não adaptada (facultativo no enunciado):** o enunciado
  diz que se *pode* adaptar `compilador_c/rt/runtime.asm` para usar as
  chamadas de sistema. A escolha aqui é não adaptar, por três motivos: (1) o
  `runtime.asm` é o próprio *programa* do usuário -- ele instala o quadro 0
  e chama `main` direto, o que conflita com o boot do SO (quem cria o init é
  o kernel); (2) as primitivas de E/S dele (`mancha_out`, `mancha_in`,
  `mancha_halt`) usam `out`/`in`/`halt` diretos, e a CPU recusa `in`/`out`
  com `I=0` (exceção 2, "instrução privilegiada") -- o quadro do processo de
  usuário tem `I=0` por exigência do enunciado; (3) para usar a biblioteca
  seria preciso uma variante do runtime com entrada `_start` própria e com
  essas três primitivas roteadas para o `trap 7` do SO. Como a adaptação é
  facultativa e os programas entregues já chamam o SO diretamente (pela
  ponte `so_chama`), a entrega fica com o escopo menor e com comportamento
  verificável: todas as saídas de console do trabalho passam pelas chamadas
  de sistema, e é por isso que o relatório só existe para quem tem processo
  rodando.

## 2. Configurações avaliadas

Cada linha é um binario `out-P<politica>-Q<quantum>-I<intervalo>-C<carga>.mob`.

| Parametro | Valores                                                                                                                                                                     |
| --------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| política | 0 = circular, 1 = prioridade                                                                                                                                                |
| quantum   | 1, 2, 4, 8, 16 (em interrupcoes do relógio)                                                                                                                                |
| intervalo | 1000, 2000, 4000 (instruções por tick)                                                                                                                                    |
| carga     | 0 = 1 CPU-bound, 1 = 1 bloqueante, 2 = 1 misto, 3 = 1 leitor de console, 4 = 4 CPU-bound iguais, 5 = 4 CPU-bound de cargas dif., 6 = puro + bloqueante + misto concorrentes |

Alvos do `Makefile` (exemplos): `make run-rr-q4`, `make run-prio-q4`,
`make run-rr-q1`, `make run-rr-q16`, `make run-i1000`,
`make run-conc-rr-q4`, `make run-conc-prio-q4`, `make run-hetr-rr-q4`,
`make run-hetr-prio-q4`, `make run-mix-rr-i1000`, `make run-mix-prio-i1000`,
`make run-cpu-i1000`, `make run-cpu-i2000`, `make run-cpu-i4000`.
Saídas completas em `parte3/resultados/`.

> `make clean && make` gera o `parte3/t2.mob` com a configuração padrão do
> `Makefile`: **POLITICA 0, QUANTUM 4, CARGA 2, INTERVALO 4000** -- a
> mesma do `build/exp/out-P0-Q4-I4000-C2.mob` usado como referencia neste
> relatório (regeneravel com `make run-rr-q4`).

## 3. Resultados

### 3.1 Quantum (circular, carga mista, intervalo 4000)

| quantum | preempções | SO t (instr) | irq relógio | E de p2 | instr simulador |
| ------- | ------------ | ------------ | ------------ | ------- | --------------- |
| 1       | 100          | 400000       | 100          | 96      | 480882          |
| 2       | 29           | 260000       | 65           | 34      | 337365          |
| 4       | 11           | 224000       | 56           | 17      | 302911          |
| 8       | 5            | 208000       | 52           | 10      | 288301          |
| 16      | 0            | 204000       | 51           | 7       | 282095          |

Quantum menor => mais expirações do quantum e mais entradas em execução:
`E` de p2 cresce de 7 para 96. Esta comparação mede eventos e tempo total;
não mede diretamente a latência de resposta.

### 3.2 Intervalo do relógio (circular, quantum 4, carga mista)

| intervalo | SO t (instr) | irq relógio | resolução visível | instr simulador |
| --------- | ------------ | ------------ | -------------------- | --------------- |
| 1000      | 397000       | 397          | p4 e=2000, p=1000    | 595376          |
| 2000      | 292000       | 146          | p4 e=4000, p=2000    | 379241          |
| 4000      | 224000       | 56           | p3 e=4000            | 302911          |

O mesmo trabalho leva `SO t` diferente conforme o intervalo. A leitura mais simples é o **custo do tratador de relógio** (a seção 5 explica por que os dados não separam isso de tick fundido).
Modelando `T(ticks) = W + c*ticks` (W = trabalho, c = custo por interrupção)
e usando os ticks = `SO t / intervalo` (que batem com a coluna *irq relógio*),
as linhas I1000 e I4000 dão

    W + 397c = 397000
    W +  56c = 224000   =>  341c = 173000  =>  c ~ 507 instruções/tick

Com W ~ 196000, o ponto I2000 (`W + 146c ~ 270000`) fica cerca de 8% abaixo
  do medido (292000). Ou seja, **cada interrupção de relógio custa cerca de 500
  instruções** (na carga 0, isolando boot/relatório, sai ~300 -- ver seção 5),
  e com intervalo 1000 isso é ~metade da CPU. Intervalo menor dá resolução
  mais fina das métricas (`p4 e=2000`), ao custo de gastar mais tempo em
  tratador. A seção 5 separa essa parcela da parcela constante (boot +
  impressão do relatório).

### 3.3 Circular x Prioridade

**Carga concorrente homogênea** (4 processos CPU-bound iguais, quantum 4):
ambos dao fatias quase iguais (`pre=6` cada; no circular `e=100000` em três
processos e `96000` no quarto, e na prioridade `e=100000` nos quatro -- um
processo que termina com a fila quase vazia perde o resto do quantum);
`SO t` = 412000 no circular e 424000 na prioridade (diferença de ~3%). Como os
processos são idênticos, `t_exec/t_quantum ~= 1` para todos e a prioridade
converge para o mesmo valor; nesse caso se comporta como o circular, conforme
esperado pela fórmula.

**Carga concorrente heterogênea** (4 processos CPU-bound de cargas
diferentes: 500/1500/3000/6000 iterações, quantum 4):

|                      | circular | prioridade |
| -------------------- | -------- | ---------- |
| p5 (maior carga) pre | 12       | 12         |
| p5 e (instr)         | 200000   | 196000     |
| pre total            | 22       | 22         |
| resposta do init (r) | 24000    | 0          |

A preempção do processo mais longo é igual nas duas políticas (12); a
prioridade altera ligeiramente seu tempo de execução e a resposta do init
(24000 para 0). A diferença é pequena: as cargas 4 e 5 são CPU-bound puras,
então todos partem de prioridade 0,5 e a fórmula (uma média) converge
lentamente. O cenário onde a prioridade deve vencer é a **mistura de
CPU-bound com processos que se bloqueiam** -- é o que a CARGA 6 mede.

**Mistura concorrente** (CARGA 6: `proc_puro(8000)` + `proc_bloq(8)` +
`proc_misto(4000)` criados juntos; quantum 4, intervalo 1000 para obter
resolução quatro vezes mais fina no tempo de resposta. O efeito também
aparece com intervalo 4000):

| métrica                                           | circular | prioridade      |
| -------------------------------------------------- | -------- | --------------- |
| `SO t` total (instr)                             | 1158000  | 1197000         |
| resposta do bloqueante (p3, `r`)                 | 7000     | **0**     |
| resposta do misto (p4, `r`)                      | 4000     | 1000            |
| espera média dos filhos curtos (`p` de p5..p20) | ~7100    | **~1560** |
| preempções totais                                | 276      | 283             |

O mecanismo não é o CPU-bound "ceder a vez": um processo em execução só
perde a CPU no fim do quantum ou ao se bloquear. O que a prioridade muda é
**quem é escolhido em cada escalonamento**. Quem usa pouco quantum
(bloqueantes e filhos curtos, que morrem rápido) acumula prioridade alta e é
escolhido antes do CPU-bound; no circular, todos esperam a vez na fila. Por
isso a espera dos filhos curtos cai de ~7100 para ~1560 e o bloqueante
entra e já roda (`r=0`).

Quanto ao resto da tabela: a resposta do misto difere em 3 ticks
(1000 vs 4000) nesta execução; a resolução e o escalonamento alteram essa
métrica, então o resultado principal é a melhora dos processos curtos e do
bloqueante. O `SO t` total difere ~3% (1197000 vs 1158000 com intervalo
1000), mas o **sinal inverte** com intervalo 4000 (624000 sob prioridade vs
636000 no circular, -2%): a diferença de tempo total entre as políticas é
pequena e sem sinal consistente, então **não a atribuo ao custo de calcular
a prioridade**. O resultado que sustenta o comparativo é a espera dos filhos
curtos e o `r` do bloqueante. Com `INTERVALO=4000` o efeito também aparece,
com resolução mais grossa: `r` do bloqueante 24000 -> 0 e espera média dos
filhos ~20000 -> ~4500 (ver `out-P0-Q4-I4000-C6.mob.txt` e
`out-P1-Q4-I4000-C6.mob.txt` em `parte3/resultados/`).

**Resultados desfavoráveis à prioridade.** O ganho da prioridade se concentra nos processos curtos e bloqueantes; o custo
recai sobre o CPU-bound e não é uniforme entre configurações:

- Carga 6, intervalo 4000: o tempo de retorno do CPU-bound (`p2`) sobe de
  488000 (circular) para 620000 (prioridade), cerca de +27%, e a resposta do
  misto (`p4`, `r`) piora de 8000 para 12000. Em compensação o bloqueante
  (`p3`) cai de `v=464000` para `v=108000` e o misto termina antes
  (`v` 628000 -> 576000). Com intervalo 1000 o efeito sobre o CPU-bound é
  pequeno (`v` 1150000 -> 1189000, +3%) e a resposta do misto melhora
  (4000 -> 1000): o sinal depende da resolução do relógio.
- Carga 4 (4 CPU-bound iguais), quantum 1: `SO t` sobe de 772000 (193 ticks)
  para 860000 (215 ticks), cerca de +11%. Com quantum 1 o escalonador roda a
  cada tick (`pre` = `irq r` nas duas políticas), então qualquer custo por
  escalonamento aparece por inteiro. Não isolei a causa; a aritmética da
  prioridade usa `mul`/`div` nativos do Mancha, então não é divisão em
  software.

A vantagem da prioridade é, portanto, específica de cargas que misturam
processos curtos/bloqueantes com CPU-bound.

### 3.4 Cargas

| carga | descrição                       | SO t (instr) | ocioso | preempções |
| ----- | --------------------------------- | ------------ | ------ | ------------ |
| 0     | 1 CPU-bound (`proc_puro 3000`)  | 104000       | 0      | 6            |
| 1     | 1 bloqueante (`proc_bloq 5`)    | 44000        | 0      | 0            |
| 2     | 1 misto (`proc_misto 3000`)     | 224000       | 0      | 11           |
| 3     | 1 leitor de console (`proc_le`) | 72000        | 64000  | 0            |
| 4     | 4 CPU-bound iguais                | 412000       | 0      | 24 (total)   |
| 5     | 4 CPU-bound de cargas dif.        | 384000       | 0      | 22 (total)   |
| 6     | puro + bloqueante + misto         | 636000       | 0      | 32 (total)   |

As evidências cobrem 28 combinações selecionadas (incluindo a execução
interativa da carga 3), não o produto cartesiano completo dos parâmetros.
Com uma única carga (0-3) o ocioso só fica alto na carga 3, onde o único
processo de usuário (o `proc_le`) está bloqueado esperando a console -- o SO
roda o processo ocioso, não faz espera ocupada. As cargas 4-6 criam vários
processos concorrentes para exercitar a preempção e a diferença entre
políticas (números em 3.3). A carga 3 é interativa, então o tempo ocioso
depende do atraso antes da entrada; nesta execução foi 64000. Os valores da
carga 6 nesta tabela são com intervalo 4000; com intervalo 1000 (3.3) são
`SO t`=1158000 e 276 preempções sob circular.

### 3.5 Parte I e Parte II (testes dedicados)

**Parte I** (gerar com `make run-p1` e executar o binário conforme
`parte1/SAIDA_ESPERADA.txt`):

```
A pids: 2 3 4          <- pids monotonos (numero de serie)
B tabela: cheio(-1) n=7 <- 7 processos cabem; o 8o retorna -1
C erros: inexist=-1, auto=-1  <- espera de pid inexistente e do proprio pid
```

Relatório (com tempos em resolução de 1 instrução):
```
ini p1
A pids: 2 3 4
B tabela: cheio(-1) n=7
C erros: inexist=-1, auto=-1
fim
tempo = ticks x intervalo de relogio, em instrucoes
intervalo do relogio = 4000 instrucoes por tick
p2 v=4000 e=3093 p=0 b=0 pre=0 r=- E=1 P=1 B=0
p3 v=4000 e=1787 p=0 b=0 pre=0 r=- E=1 P=1 B=0
...
SO t=304000 ocioso=0 irq r=76 s=105 c=0 x=0 pre=17 proc=11
irq por vetor: v7=105 v10=76
```

*Nota: valores de `e`, `p`, `b`, `SO t` agora em resolução de 1 instrução (antes eram múltiplos de 4000).*

**Parte II** (gerar com `make run-p2` e executar o binário conforme
`parte2/SAIDA_ESPERADA.txt`): dois processos leem a console e ficam
bloqueados; o SO roda o processo ocioso enquanto isso; a chegada de entrada
desbloqueia os dois (cada um imprime o caractere lido).
Depois de D0 e C, digitar `Eab` e Enter para enfileirar dois caracteres
imprimíveis e destravar os dois leitores. O comando `E` acrescenta uma
quebra de linha ao texto; `Ea` seguido de `Eb` faria o segundo leitor receber
a quebra de linha em vez de `b`.

```
espera vivo=0
le:le: [a]
 [b]
SO t=112000 ocioso=76000 irq r=28 s=53 c=1 x=0 pre=0 proc=4
irq por vetor: v7=53 v8=1 v10=28
```

`ocioso=76000` (≈68% do tempo) mostra que o SO não faz espera ocupada: com
os dois leitores bloqueados e o init esperando, o único processo pronto é o
ocioso, e o SO o roda. `v8=1` confirma que a entrada chegou por interrupção
da console e desbloqueou os leitores.

**Ressalva:** esse tempo ocioso é atraso de digitação (quanto tempo o
teste ficou parado antes da entrada `Eab`), não uma propriedade do SO
-- por isso o percentual varia de execução para execução. O que ele prova é
apenas isto: **nenhum processo de usuário fica girando em espera ocupada**
enquanto bloqueado. O próprio processo ocioso gira num `for(;;)`, o que é
esperado (é o que se quer que rode quando não há mais nada para fazer).

### 3.6 Exceções em processos filhos (gerar com `make run-excecoes` e executar conforme

`parte3/SAIDA_ESPERADA.txt`)

Processos que executam instrução privilegiada/divisão por zero são mortos
individualmente (não derrubam o sistema):

```
EXCECAO v2 pid=2 ip=25290
EXCECAO v3 pid=3 ip=25304
ok
SO ... x=2 ... v2=1 v3=1
irq por vetor: v2=1 v3=1 v7=24 v10=22
```

Um processo normal (`proc_ok`) imprime `ok` depois das exceções; o init
termina com `fim`. Os vetores `v2` e `v3` registram exatamente uma exceção
cada.

### 3.7 Exceção no init (gerar com `make run-init-excecao`)

Este teste separado faz o próprio init executar `outb` em modo usuário. A
exceção v2 deve matar o init e imprimir o relatório antes de parar o SO:

```
I
EXCECAO v2 pid=1 ip=24796
...
p1 v=0 e=0 p=0 b=0 pre=0 r=- E=1 P=1 B=0
SO t=0 ocioso=0 irq r=0 s=1 c=0 x=1 pre=0 proc=1
irq por vetor: v2=1 v7=1
```

O marcador `X`, escrito pela instrução seguinte à chamada assembly que falha,
não aparece. `SO t=0` é coerente com a resolução de um tick: a exceção ocorre
antes da primeira interrupção do relógio. Esse caso confirma por execução
que o relatório é impresso quando o init morre por exceção, não apenas quando
ele chama `SO_MATA_PROC`.

## 4. Análise

- O escalonador circular preserva a ordem FIFO e reparte igualmente a CPU
  entre cargas CPU-bound concorrentes. Reduzir o quantum aumenta as
  expirações e as entradas em execução, conforme a seção 3.1.
- A política por prioridade não muda o quantum: ela muda qual processo pronto
  é escolhido quando ocorre um escalonamento. Por isso, as cargas homogêneas
  se comportam de forma semelhante, enquanto na carga 6 os processos
  interativos e curtos esperam menos. O tempo total não mostra vantagem
  consistente de uma política sobre a outra (seção 3.3).
- Na carga 3, o leitor bloqueado deixa a CPU disponível para o processo
  ocioso. O tempo medido depende de quando a entrada é fornecida; o resultado
  demonstra o bloqueio, não um tempo ocioso fixo.
- As exceções de usuário encerram apenas o processo culpado. Os testes dos
  filhos e do próprio init confirmam que o relatório é impresso nos dois
  caminhos.

## 5. Limitações

- **Resolução de tempo = 1 instrução.** O SO agora combina `ticks * intervalo + contador_do_relogio` (porta 0x20), atingindo resolução de 1 instrução. A multiplicação e soma são feitas em base 100 (32 bits simulados) para não estourar o `int` de 16 bits.
- **O tempo reportado agora é muito próximo do tempo de parede do simulador.** Com a medição em 32 bits (`ticks * intervalo + contador`), `SO t` fica a ~1.700 instruções do valor real (boot antes de o relógio ser programado). Em `t2p1`: SO t = 304.000 vs 302.911 real (erro ~0,4%). Em `t2.mob`: SO t = 224.000 vs 224.000 real (exato). Processos idênticos agora mostram tempos similares (ex: `t2p1` p3 e=3093, p4 e=1787 em vez de 4000 vs 0).
- **E e pre medem eventos diferentes.** `E` conta entradas em execução;
  `pre` conta toda expiração do quantum, inclusive quando o mesmo processo
  volta imediatamente a executar por ser o único pronto. Bloqueios e mortes
  não contam como preempção.
- **`SO_ESCREVE` não bloqueia.** O enunciado (parte II) pede bloqueio quando o
  dispositivo não está pronto. O simulador devolve sempre 1 no bit 0 da porta
  0x02 (`simulador_completo/src/dispositivos.c`, leitura de
  `PORTA_CONSOLE_ESTADO`: "bit0: sempre pronta para enviar"), então a escrita
  é imediata e um bloqueio em escrita nunca seria acionado; só `SO_LE` bloqueia
  de fato. Se o enunciado exigir a letra, o ajuste é ler o bit 0 da porta 0x02
  na ponte e bloquear com um novo motivo de bloqueio, guardando o caractere
  pendente em `Processo` para `so_trata_pendencias` escrever.
- **`N_REGISTROS`=256.** O relatório guarda métricas por processo, uma entrada
  por processo encerrado. A tabela foi ampliada de 64 para 256 para que a
  contagem de processos criados ao longo de uma execução (é o que limita, e
  não o número de processos vivos) não aperte a tabela: as cargas deste
  relatório criam no máximo 20 processos, mas uma carga mais longa não ficaria
  limitada pela métrica. Se ainda assim a tabela encher, os excedentes são
  descartados e o relatório emite um aviso de descarte (não silencioso).
- Na console, a BIOS gera **uma** interrupção mesmo que várias teclas cheguem
  juntas; por isso `c` pode ser menor que o número de caracteres digitados.
- Não há detecção de espera circular (deadlock) entre `SO_ESPERA_PROC`.
- Exceções ocorridas no próprio kernel (frame com S=1) geram pânico e param o
  simulador; elas não são atribuídas ao processo que estava interrompido. O
  tratamento e a remoção individual cobrem exceções em modo usuário.
- **Processos vivos entram no relatório.** Quando o init morre, `arquiva_vivos`
  fecha o tempo do estado atual de cada filho ainda vivo e o arquiva com a
  marca `(vivo)`. Testado em `parte3/t2vivos.mob` (`make run-vivos`): init
  cria um filho em laço e outro bloqueado em `SO_LE` e morre; ambos aparecem.
- **Processo ocioso em modo supervisor** (`sr=0xA000`). É de propósito
  seguro: se ele rodasse com S=0 e falhasse, `so_excecao` usaria
  `processo_atual = -1` como índice (`processos[-1]`); em S=1 a falha cai no
  ramo "PANICO". O laço `for(;;){}` não executa nada que possa falhar.
- **Pilhas de 64 palavras por processo, sem verificação de estouro.** O
  enunciado do T2 não exige proteção de memória; os programas entregues usam
  poucas palavras de pilha.

## 6. Como reproduzir

### Pré-requisitos
- WSL2 com Ubuntu (ou Linux nativo)
- Diretório do projeto com as 4 pastas: `t2/`, `bios_mancha/`, `compilador_c/`, `simulador_completo/`

---

### 1. Build completo (do zero)

```bash
cd t2
make clean && make
```

**O que acontece:**
- Recompila `mcc` localmente (precisa GLIBC 2.38, o sistema tem 2.35)
- Copia `montador` e `simulador` dos binários originais
- Compila `kernel.c` e todos os programas de usuário
- Monta os **6 binários finais** em `parte1/`, `parte2/`, `parte3/`

**Verificação:**
```bash
ls -la parte1/ parte2/ parte3/
# Deve mostrar 6 arquivos .mob:
# parte1/t2p1.mob
# parte2/t2p2.mob
# parte3/t2.mob, t2ex.mob, t2initex.mob, t2vivos.mob
```

---

### 2. Simulador largo (para ler relatório completo)

```bash
make simulador-largo
```
Gera `build/simulador_largo` (console 60 linhas vs 6 padrão).

---

### 3. Executar os 6 testes (ordem recomendada)

> **Formato padrão:** `TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo <arquivo.mob>`
> No simulador: **`D0`** (velocidade máxima) → **`C`** (continua)

---

#### TESTE 1 — Parte 1 (Processos Básicos)
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte1/t2p1.mob
# No simulador: D0 + C
```
**Saída esperada (`parte1/SAIDA_ESPERADA.txt`):**
```
A pids: 2 3 4
B tabela: cheio(-1) n=7
C erros: inexist=-1, auto=-1
fim
SO t=304000 ... (resolução 1 instrução)
```

---

#### TESTE 2 — Parte 2 (Bloqueio e E/S) — **INTERATIVO**
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte2/t2p2.mob
# No simulador: D0 + C
# Aguarda aparecer "le:" (leitores bloqueados)
# Digite: Eab + Enter
```
**Saída esperada:**
```
espera vivo=0
le:le: [a]
 [b]
fim
SO t=... ocioso=... v8=1 ...
```

---

#### TESTE 3 — Parte 3A: Circular Padrão
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte3/t2.mob
# D0 + C
```
**Saída:**
```
ini
......fim
SO t=224000 ... p2 pre=11 ... p1 b=220000
```

---

#### TESTE 4 — Parte 3B: Exceções em Filhos
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte3/t2ex.mob
# D0 + C
```
**Saída:**
```
excecoes
EXCECAO v2 pid=2 ip=25290
EXCECAO v3 pid=3 ip=25304
ok
fim
```

---

#### TESTE 5 — Parte 3C: Exceção no Init
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte3/t2initex.mob
# D0 + C
```
**Saída:**
```
I
EXCECAO v2 pid=1 ip=24796
...
SO t=0 ... (marcador X NÃO aparece)
```

---

#### TESTE 6 — Parte 3D: Processos Vivos no Relatório
```bash
TERM=xterm-256color COLUMNS=120 LINES=50 build/simulador_largo parte3/t2vivos.mob
# D0 + C
```
**Saída:**
```
f
...
p2 ... (vivo)
p3 ... (vivo)
```

---

### 4. Comparar com saídas esperadas

```bash
cat parte1/SAIDA_ESPERADA.txt
cat parte2/SAIDA_ESPERADA.txt
cat parte3/SAIDA_ESPERADA.txt
```

---

### 5. Experimentos (Parte III - 27 configurações)

```bash
# Exemplos:
make run-rr-q4      # Circular, quantum 4, intervalo 4000, carga 2
make run-prio-q4    # Prioridade, quantum 4, intervalo 4000, carga 2
make run-rr-q1      # Quantum 1
make run-rr-q16     # Quantum 16
make run-i1000      # Intervalo 1000
make run-cpu        # Carga 0 (1 CPU-bound)
make run-bloq       # Carga 1 (bloqueante)
make run-mista      # Carga 2 (misto)
make run-conc-rr-q4 # Carga 4 (4 CPU-bound)
make run-hetr-rr-q4 # Carga 5 (heterogênea)
make run-mix-rr-i1000 # Carga 6 (mista) + intervalo 1000
```
Cada `make run-*` gera `build/exp/out-P*-Q*-I*-C*.mob` — execute com `build/simulador_largo`.

---

### Limpeza

```bash
make clean      # Remove apenas build/ (preserva binários entregues)
make distclean  # Remove tudo, incluindo os 6 .mob entregues
```

---

### Notas Importantes

| Item | Detalhe |
|------|---------|
| **Simulador** | `build/tools/simulador` (console 6 linhas) — use `make simulador-largo` para versão estendida (60 linhas) apenas para leitura |
| **Variáveis de ambiente** | `TERM=xterm-256color COLUMNS=120 LINES=50` necessárias no WSL/Linux para ncurses |
| **Comandos no simulador** | `D0` = velocidade máxima, `C` = continua, `E<texto>` = entrada console |
| **Parte 2 (interativa)** | Após `C`, digite `Eab` + Enter para fornecer entrada aos leitores |
| **Métricas** | Resolução = **1 instrução** (ticks × intervalo + contador hardware 0x20) |
| **Determinismo** | 27 configs não interativas = reprodutíveis; Parte 2 e carga 3 = variam com tempo de digitação |

---

### Estrutura de Entrega

```
t2/
├── Makefile
├── RELATORIO.md
├── src/              # kernel.c, ponte.asm (fonte única)
├── usuario/          # programas de usuário
├── parte1/           # t2p1.mob + SAIDA_ESPERADA.txt + resultados/
├── parte2/           # t2p2.mob + SAIDA_ESPERADA.txt + resultados/
├── parte3/           # t2.mob, t2ex.mob, t2initex.mob, t2vivos.mob + SAIDA_ESPERADA.txt + resultados/
├── build/            # GERADO (não entregue)
├── historico/        # fontes ao final da Parte II (não participa do build)
└── *.mob (6 arquivos em parte1/, parte2/, parte3/)  # ENTREGUES
```

---

### Reprodutibilidade

> **Recompilar com estas fontes e executar reproduz os mesmos números** (o `.mob` sai idêntico byte a byte ao entregue).

```bash
make clean && make
# Todos os 6 .mob saem idênticos byte a byte aos entregues
```

Execuções interativas (Parte II e carga 3) variam no tempo ocioso conforme o atraso na digitação da entrada (`Eab`).

## 7. Matriz de rastreabilidade (enunciado -> implementação -> evidência)

| Requisito                           | Onde                                                    | Evidência                                                                                                      |
| ----------------------------------- | ------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| struct/tabela de processos          | `src/kernel.c` `struct Processo`, `processos[]`   | `parte1/resultados/t2p1.txt`                                                                                  |
| processo atual com "nenhum"         | `processo_atual = -1`                                 | `parte1/resultados/t2p1.txt`                                                                                  |
| salvar/restaurar quadro 16 palavras | `salva_quadro`/`restaura_quadro`                    | resultados das três partes                                                                                     |
| pid != slot                         | `proximo_pid`                                         | `parte1/resultados/t2p1.txt`                                                                                  |
| SO_LE bloqueia / pendencia          | `so_le`, `so_trata_pendencias`                      | `parte2/resultados/t2p2.txt`                                                                                  |
| SO_ESCREVE imediato                 | `so_escreve`                                          | resultados das três partes                                                                                     |
| SO_CRIA_PROC/SO_MATA_PROC           | `so_cria_proc`/`so_mata_proc`                       | `parte1/resultados/t2p1.txt`                                                                                  |
| SO_ESPERA_PROC + erros              | `so_espera_proc`                                      | `parte1/resultados/t2p1.txt`                                                                                  |
| circular com quantum                | `so_escalona`/`so_relogio`                          | `parte3/resultados/out-*.txt`                                                                                 |
| prioridade                          | `so_escalona`                                         | `parte3/resultados/out-*.txt`                                                                                 |
| métricas do sistema                | `so_relatorio`                                        | resultados das três partes                                                                                     |
| métricas por processo              | `struct Registro`                                     | resultados das três partes                                                                                     |
| tempo em 32 bits (1 instr)         | `tempo_instrucoes`, `kernel_tempo32`, `_f_le_contador_relogio` | `parte1/resultados/t2p1.txt` (SO t=304000 vs 302911 real)                                                      |
| relatório quando o init morrer     | ramo de `pid_init` em `so_mata_proc`/`so_excecao` | `parte3/resultados/t2initex.txt` testa a morte do init por exceção; `t2ex.txt` testa exceções em filhos |
| processos vivos no relatório        | `arquiva_vivos` em `so_relatorio`                      | `parte3/resultados/t2vivos.txt`                                                                                |
| contagem por vetor de interrupção | `int_tipo[16]`                                        | resultados das três partes                                                                                     |
| exceções matam só o culpado      | `so_excecao`                                          | `parte3/resultados/t2ex.txt`                                                                                  |