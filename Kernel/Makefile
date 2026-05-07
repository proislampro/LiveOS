KERNEL     = build/kernel.elf
KERNELE    = src/entry.s
KERNELC    = src/main.c
BOOTO      = build/boot.o
KERNELO    = build/kernel.o

CC = gcc
AS = nasm
LD = ld

CFLAGS  = -ffreestanding -m64 -O2 -Wall -Wextra -fno-pic -fno-stack-protector -mno-red-zone -mcmodel=kernel -nostdlib -g -Og -mcmodel=kernel -I ../Utilities -I src/Drivers/Headers
LDFLAGS = -m elf_x86_64 -T src/linker.ld -nostdlib

all: $(KERNEL)

$(KERNEL): compile assemble
	@echo "  $$  Linking    --> Kernel/build/$(notdir $(KERNEL))"
	@$(LD) $(LDFLAGS) -o $(KERNEL) $(BOOTO) $(KERNELO)
	@rm -f $(KERNELO) $(BOOTO)

compile:
	@echo "  $$  Compiling  --> Kernel/build/$(notdir $(KERNELO))"
	@$(CC) $(CFLAGS) -c $(KERNELC) -o $(KERNELO)
	@echo "  $$  Compiling  --> Kernel/build/drivers/ata.o"
	@$(CC) $(CFLAGS) -c src/drivers/ata.c -o build/drivers/ata.o
	@echo "  $$  Compiling  --> Kernel/build/drivers/fat32.o"
	@$(CC) $(CFLAGS) -c src/drivers/fat32.c -o build/drivers/fat32.o
	@echo "  $$  Compiling  --> Kernel/build/drivers/keyboard.o"
	@$(CC) $(CFLAGS) -c src/drivers/keyboard.c -o build/drivers/keyboard.o
	@echo "  $$  Compiling  --> Kernel/build/drivers/screen.o"
	@$(CC) $(CFLAGS) -c src/drivers/screen.c -o build/drivers/screen.o
	@echo "  $$  Compiling  --> Kernel/build/drivers/mouse.o"
	@$(CC) $(CFLAGS) -c src/drivers/mouse.c -o build/drivers/mouse.o

assemble:
	@echo "  $$  Assembling --> Kernel/build/$(notdir $(BOOTO))"
	@$(AS) -f elf64 $(KERNELE) -o $(BOOTO)

clean:
	@echo "  $$  Cleaning   --> Kernel/build/"
	@rm -f $(KERNELO) $(BOOTO) $(KERNEL)

.PHONY: all clean