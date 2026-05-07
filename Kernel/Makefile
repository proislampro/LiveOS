KERNEL = Build/kernel.elf

CC = gcc
AS = nasm
LD = ld


C_SOURCES   := $(shell find src -name "*.c")
ASM_SOURCES := $(shell find src -name "*.asm" -o -name "*.s")

# Convert source paths to object paths
C_OBJECTS   := $(patsubst src/%.c,Build/Obj/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst src/%.asm,Build/Obj/%.o,$(filter %.asm,$(ASM_SOURCES)))
ASM_OBJECTS += $(patsubst src/%.s,Build/Obj/%.o,$(filter %.s,$(ASM_SOURCES)))

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


Build/Obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "  $$  Compiling  --> $@"
	@$(CC) $(CFLAGS) -c $< -o $@


Build/Obj/%.o: src/%.asm
	@mkdir -p $(dir $@)
	@echo "  $$  Assembling --> $@"
	@$(AS) -f elf64 $< -o $@


Build/Obj/%.o: src/%.s
	@mkdir -p $(dir $@)
	@echo "  $$  Assembling --> $@"
	@$(AS) -f elf64 $< -o $@


clean:
	@echo "  $$  Cleaning   --> Build/" 
	@rm -rf Build/*

.PHONY: all clean