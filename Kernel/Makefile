KERNEL = build/kernel.elf

CC = gcc
AS = nasm
LD = ld


C_SOURCES   := $(shell find src -name "*.c")
ASM_SOURCES := $(shell find src -name "*.asm" -o -name "*.s")

# Convert source paths to object paths
C_OBJECTS   := $(patsubst src/%.c,build/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst src/%.asm,build/%.o,$(filter %.asm,$(ASM_SOURCES)))
ASM_OBJECTS += $(patsubst src/%.s,build/%.o,$(filter %.s,$(ASM_SOURCES)))

OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)


CFLAGS = \
	-ffreestanding \
	-m64 \
	-O2 \
	-Wall \
	-Wextra \
	-fno-pic \
	-fno-stack-protector \
	-mno-red-zone \
	-mcmodel=kernel \
	-nostdlib \
	-g \
	-Og \
	-I ../Utilities \
	-I src/Drivers/Headers \
	-I src/Core/Headers \
	-I src/System/Headers \
	-I include

LDFLAGS = \
	-m elf_x86_64 \
	-T src/linker.ld \
	-nostdlib


all: $(KERNEL)

$(KERNEL): $(OBJECTS)
	@echo "  $$  Linking    --> $(KERNEL)"
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)


build/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "  $$  Compiling  --> $@"
	@$(CC) $(CFLAGS) -c $< -o $@


build/%.o: src/%.asm
	@mkdir -p $(dir $@)
	@echo "  $$  Assembling --> $@"
	@$(AS) -f elf64 $< -o $@


build/%.o: src/%.s
	@mkdir -p $(dir $@)
	@echo "  $$  Assembling --> $@"
	@$(AS) -f elf64 $< -o $@


clean:
	@echo "  $$  Cleaning   --> build/" 
	@rm -rf build/*

.PHONY: all clean