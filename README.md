# T1 — Número pseudoaleatório em assembly Mancha

Programa em linguagem de montagem do processador Mancha que gera um número
pseudoaleatório de 16 bits e o imprime em 4 dígitos hexadecimais a cada
execução (ex.: `BF6D`).

## Arquivos

- `aleatorio.asm` — código-fonte
- `aleatorio.mob` — código objeto montado (é o que o simulador executa)

## Como executar

A partir da pasta `simulador_completo/`:

```sh
./bin/montador ../T1/aleatorio.asm -o ../T1/aleatorio.mob
./bin/simulador ../T1/aleatorio.mob
```

Requer terminal de pelo menos 90 colunas x 36 linhas. No simulador, `C`
executa e `F` sai.

## Como funciona

Lê o contador do relógio (portas `0x20`/`0x21`) como semente e aplica
`x = (x * 25173 + 13849) mod 65536`; imprime o resultado em hexadecimal.
