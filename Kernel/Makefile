KERNEL     = build/kernel.elf
KERNELE    = src/kernel_entry.s
KERNELC    = src/kernel.c
BOOTO      = build/boot.o
KERNELO    = build/kernel.o

CC = gcc
AS = nasm
LD = ld

CFLAGS  = -ffreestanding -m64 -O2 -Wall -Wextra -fno-pic -fno-stack-protector -mno-red-zone -mcmodel=kernel -nostdlib -g -Og
LDFLAGS = -m elf_x86_64 -T src/linker.ld -nostdlib

all: $(KERNEL)

$(KERNEL): $(KERNELO) $(BOOTO)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(BOOTO) $(KERNELO)
	@rm -f $(KERNELO) $(BOOTO)

$(KERNELO): $(KERNELC)
	@$(CC) $(CFLAGS) -c $(KERNELC) -o $(KERNELO)

$(BOOTO): $(KERNELE)
	@$(AS) -f elf64 $(KERNELE) -o $(BOOTO)

clean:
	@rm -f $(KERNELO) $(BOOTO) $(KERNEL)

.PHONY: all clean