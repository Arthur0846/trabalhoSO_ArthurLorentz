# T2 -- Sistema operacional de processos para o Mancha.
#
# Estrutura (ver RELATORIO.md):
#   src/        kernel e ponte; fonte unica das tres partes
#   usuario/    cargas e testes de usuario
#   parte1..3/  binarios entregues, saidas esperadas e resultados
#   historico/  fontes ao final da parte II; nao participam do build
#   build/      artefatos gerados; nao integram a entrega
#
# Execute os alvos a partir de t2/. Os caminhos ../ apontam para o projeto
# Mancha. O build nao altera ../compilador_c/bin nem ../simulador_completo/bin.

BIOS     = ../bios_mancha/src/bios.asm
SIM_SRC  = ../simulador_completo/src

SRC   = src
USR   = usuario
BUILD = build
TOOLS = $(BUILD)/tools
MCC      = $(TOOLS)/mcc
MONTADOR = $(TOOLS)/montador
SIM      = $(TOOLS)/simulador
EXP   = $(BUILD)/exp

# Ferramentas locais: mcc recompilado dos fontes originais; montador e
# simulador copiados dos binarios originais. Apenas as copias recebem +x.
MCC_SRC  = ../compilador_c/src/pre.c ../compilador_c/src/lexer.c \
           ../compilador_c/src/tipos.c ../compilador_c/src/simbolos.c \
           ../compilador_c/src/parser.c ../compilador_c/src/main.c
MCC_HDR  = $(wildcard ../compilador_c/src/*.h)
SIM_BIN  = ../simulador_completo/bin/simulador
MONT_BIN = ../simulador_completo/bin/montador

POLITICA  ?= 0
QUANTUM   ?= 4
CARGA     ?= 2
INTERVALO ?= 4000

all: tools parte1/t2p1.mob parte2/t2p2.mob parte3/t2.mob parte3/t2ex.mob parte3/t2initex.mob

# Diretorios gerados.
$(BUILD) $(EXP) $(TOOLS) parte1 parte2 parte3:
	mkdir -p $@

# Ferramentas locais; os binarios originais nao sao alterados.
$(TOOLS)/mcc: $(MCC_SRC) $(MCC_HDR) | $(TOOLS)
	gcc -std=gnu11 -Wall -Wextra -g -D_GNU_SOURCE -o $@ $(MCC_SRC)

$(TOOLS)/montador: $(MONT_BIN) | $(TOOLS)
	cp $< $@
	chmod 755 $@

$(TOOLS)/simulador: $(SIM_BIN) | $(TOOLS)
	cp $< $@
	chmod 755 $@

tools: $(TOOLS)/mcc $(TOOLS)/montador $(TOOLS)/simulador

# config.h e reescrito apenas quando a configuracao muda.
$(BUILD)/config.h: FORCE | $(BUILD)
	@printf '#define POLITICA %s\n#define QUANTUM %s\n#define CARGA %s\n#define INTERVALO %s\n' \
		'$(POLITICA)' '$(QUANTUM)' '$(CARGA)' '$(INTERVALO)' > $@.tmp
	@cmp -s $@.tmp $@ && rm -f $@.tmp || mv -f $@.tmp $@

FORCE:

# C -> assembly; config.h e encontrado via -I$(BUILD).
$(BUILD)/kernel.asm: $(SRC)/kernel.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

$(BUILD)/usuario.asm: $(USR)/usuario.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

$(BUILD)/usuario_excecoes.asm: $(USR)/usuario_excecoes.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

$(BUILD)/usuario_init_excecao.asm: $(USR)/usuario_init_excecao.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

$(BUILD)/teste_parte1.asm: $(USR)/teste_parte1.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

$(BUILD)/teste_parte2.asm: $(USR)/teste_parte2.c $(BUILD)/config.h $(MCC)
	$(MCC) -I$(BUILD) $< -o $@

# Binarios entregues.
parte3/t2.mob: $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario.asm $(MONTADOR) | parte3
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario.asm -o $@

parte3/t2ex.mob: $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario_excecoes.asm $(USR)/excecoes.asm $(MONTADOR) | parte3
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario_excecoes.asm $(USR)/excecoes.asm -o $@

parte3/t2initex.mob: $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario_init_excecao.asm $(USR)/init_excecao.asm $(MONTADOR) | parte3
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario_init_excecao.asm $(USR)/init_excecao.asm -o $@

parte1/t2p1.mob: $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/teste_parte1.asm $(MONTADOR) | parte1
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/teste_parte1.asm -o $@

parte2/t2p2.mob: $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/teste_parte2.asm $(MONTADOR) | parte2
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/teste_parte2.asm -o $@

run-p1: ; $(MAKE) parte1/t2p1.mob
run-p2: ; $(MAKE) parte2/t2p2.mob
run-excecoes: ; $(MAKE) parte3/t2ex.mob
run-init-excecao: ; $(MAKE) parte3/t2initex.mob

# Experimentos da parte III em build/exp/out-P*-Q*-I*-C*.mob.
NOME = out-P$(POLITICA)-Q$(QUANTUM)-I$(INTERVALO)-C$(CARGA).mob

run-rr-q4: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=2
run-prio-q4: ; $(MAKE) _run POLITICA=1 QUANTUM=4 INTERVALO=4000 CARGA=2
run-rr-q1: ; $(MAKE) _run POLITICA=0 QUANTUM=1 INTERVALO=4000 CARGA=2
run-rr-q2: ; $(MAKE) _run POLITICA=0 QUANTUM=2 INTERVALO=4000 CARGA=2
run-rr-q8: ; $(MAKE) _run POLITICA=0 QUANTUM=8 INTERVALO=4000 CARGA=2
run-rr-q16: ; $(MAKE) _run POLITICA=0 QUANTUM=16 INTERVALO=4000 CARGA=2
run-prio-q1: ; $(MAKE) _run POLITICA=1 QUANTUM=1 INTERVALO=4000 CARGA=2
run-prio-q16: ; $(MAKE) _run POLITICA=1 QUANTUM=16 INTERVALO=4000 CARGA=2
run-i1000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=1000 CARGA=2
run-i2000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=2000 CARGA=2
run-i4000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=2
run-cpu: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=0
run-bloq: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=1
run-mista: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=2
run-le: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=3
run-cpu-i1000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=1000 CARGA=0
run-cpu-i2000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=2000 CARGA=0
run-cpu-i4000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=0
run-conc-rr-q4: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=4
run-conc-prio-q4: ; $(MAKE) _run POLITICA=1 QUANTUM=4 INTERVALO=4000 CARGA=4
run-conc-rr-q1: ; $(MAKE) _run POLITICA=0 QUANTUM=1 INTERVALO=4000 CARGA=4
run-conc-prio-q1: ; $(MAKE) _run POLITICA=1 QUANTUM=1 INTERVALO=4000 CARGA=4
run-conc-rr-q16: ; $(MAKE) _run POLITICA=0 QUANTUM=16 INTERVALO=4000 CARGA=4
run-hetr-rr-q4: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=5
run-hetr-prio-q4: ; $(MAKE) _run POLITICA=1 QUANTUM=4 INTERVALO=4000 CARGA=5
run-hetr-rr-q1: ; $(MAKE) _run POLITICA=0 QUANTUM=1 INTERVALO=4000 CARGA=5
run-hetr-prio-q1: ; $(MAKE) _run POLITICA=1 QUANTUM=1 INTERVALO=4000 CARGA=5
run-mix-rr-i1000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=1000 CARGA=6
run-mix-prio-i1000: ; $(MAKE) _run POLITICA=1 QUANTUM=4 INTERVALO=1000 CARGA=6
run-mix-rr-i4000: ; $(MAKE) _run POLITICA=0 QUANTUM=4 INTERVALO=4000 CARGA=6
run-mix-prio-i4000: ; $(MAKE) _run POLITICA=1 QUANTUM=4 INTERVALO=4000 CARGA=6

# Recompila kernel e usuario; remove os objetos para garantir consistencia.
_run: $(MCC) $(MONTADOR) | $(EXP)
	@rm -f $(BUILD)/kernel.asm $(BUILD)/usuario.asm
	$(MAKE) $(BUILD)/kernel.asm $(BUILD)/usuario.asm
	$(MONTADOR) $(BIOS) $(SRC)/ponte.asm $(BUILD)/kernel.asm $(BUILD)/usuario.asm -o $(EXP)/$(NOME)
	@echo "gerado $(EXP)/$(NOME)"

# Simulador com console maior, apenas para leitura do relatorio.
# Gera uma variante a partir das fontes originais, alterando CONSOLE_LINHAS
# em tela.c. Nenhum arquivo em ../simulador_completo e alterado; a execucao
# da CPU permanece identica.
simulador-largo: $(BUILD)/simulador_largo

$(BUILD)/tela_larga.c: $(SIM_SRC)/tela.c | $(BUILD)
	sed 's/^#define CONSOLE_LINHAS .*/#define CONSOLE_LINHAS 60/' $< > $@

$(BUILD)/simulador_largo: $(BUILD)/tela_larga.c $(wildcard $(SIM_SRC)/*.c $(SIM_SRC)/*.h)
	gcc -Wall -Wextra -std=gnu11 -g -I$(SIM_SRC) -o $@ \
		$(SIM_SRC)/main.c $(SIM_SRC)/cpu.c $(SIM_SRC)/dispositivos.c \
		$(SIM_SRC)/instrucao.c $(SIM_SRC)/objeto.c $(SIM_SRC)/memoria.c \
		$(BUILD)/tela_larga.c -lcurses

# Limpeza: clean remove apenas build/, que contem artefatos regeneraveis e
# nao integra a entrega. distclean remove tambem os binarios entregues.
clean:
	rm -rf $(BUILD)

distclean: clean
	rm -f parte1/t2p1.mob parte2/t2p2.mob parte3/t2.mob parte3/t2ex.mob parte3/t2initex.mob

.PHONY: all clean distclean tools _run FORCE simulador-largo run-p1 run-p2 run-excecoes run-init-excecao \
        run-rr-q1 run-rr-q2 run-rr-q4 run-rr-q8 run-rr-q16 \
        run-prio-q1 run-prio-q4 run-prio-q16 \
        run-i1000 run-i2000 run-i4000 \
        run-cpu run-bloq run-mista run-le \
        run-cpu-i1000 run-cpu-i2000 run-cpu-i4000 \
        run-conc-rr-q4 run-conc-prio-q4 run-conc-rr-q1 run-conc-prio-q1 run-conc-rr-q16 \
        run-hetr-rr-q4 run-hetr-prio-q4 run-hetr-rr-q1 run-hetr-prio-q1 \
        run-mix-rr-i1000 run-mix-prio-i1000 run-mix-rr-i4000 run-mix-prio-i4000
