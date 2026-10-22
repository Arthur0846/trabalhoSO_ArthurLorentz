MCC = ../compilador_c/bin/mcc
MONTADOR = ../simulador_completo/bin/montador
BIOS = ../bios_mancha/src/bios.asm

all: t2.mob

kernel.asm: kernel.c
	$(MCC) kernel.c -o kernel.asm

usuario.asm: usuario.c
	$(MCC) usuario.c -o usuario.asm

t2.mob: $(BIOS) ponte.asm kernel.asm usuario.asm
	$(MONTADOR) $(BIOS) ponte.asm kernel.asm usuario.asm -o t2.mob

clean:
	rm -f kernel.asm usuario.asm t2.mob

.PHONY: all clean
