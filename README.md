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
