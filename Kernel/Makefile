KERNEL     = build/kernel.elf
KERNELE    = src/entry.s
KERNELC    = src/main.c
BOOTO      = build/boot.o
KERNELO    = build/kernel.o

CC = gcc
AS = nasm
LD = ld

CFLAGS  = -ffreestanding -m64 -O2 -Wall -Wextra -fno-pic -fno-stack-protector -mno-red-zone -mcmodel=kernel -nostdlib -g -Og -mcmodel=kernel -I ../Utilities
LDFLAGS = -m elf_x86_64 -T src/linker.ld -nostdlib

all: $(KERNEL)

$(KERNEL): $(KERNELO) $(BOOTO)
	@echo "  $$  Linking    --> Kernel/build/$(notdir $(KERNEL))"
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(BOOTO) $(KERNELO)
	@rm -f $(KERNELO) $(BOOTO)

$(KERNELO): $(KERNELC)
	@echo "  $$  Compiling  --> Kernel/build/$(notdir $(KERNELO))"
	@$(CC) $(CFLAGS) -c $(KERNELC) -o $(KERNELO)

$(BOOTO): $(KERNELE)
	@echo "  $$  Assembling --> Kernel/build/$(notdir $(BOOTO))"
	@$(AS) -f elf64 $(KERNELE) -o $(BOOTO)

clean:
	@echo "  $$  Cleaning   --> Kernel/build/"
	@rm -f $(KERNELO) $(BOOTO) $(KERNEL)

.PHONY: all clean