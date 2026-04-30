
ARCH            := x86_64
TARGET          := BOOTX64.EFI
BUILD_DIR       := build

CC              := gcc
LD              := ld
OBJCOPY         := objcopy

EFI_INC         := /usr/include/efi
EFI_LIB         := /usr/lib
EFI_CRT         := /usr/lib/crt0-efi-x86_64.o

CFLAGS := -I$(EFI_INC) -I$(EFI_INC)/$(ARCH) \
          -fpic -fshort-wchar -ffreestanding -fno-stack-protector \
          -mno-red-zone -maccumulate-outgoing-args \
          -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI

SRC := main.c
OBJ := $(BUILD_DIR)/main.o
ELF := $(BUILD_DIR)/bootloader.so
EFI := $(BUILD_DIR)/$(TARGET)

all: $(EFI)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(LD) -nostdlib -znocombreloc \
	-T /usr/lib/elf_x86_64_efi.lds \
	/usr/lib/crt0-efi-x86_64.o \
	$(OBJ) \
	-L/usr/lib -lgnuefi -lefi \
    -shared -Bsymbolic \
	-o $@

$(EFI): $(ELF)
	$(OBJCOPY) \
		-j .text -j .sdata -j .data -j .dynamic \
		-j .dynsym -j .rel -j .rela -j .reloc \
		--target=efi-app-x86_64 \
		--subsystem=10 \
		$< $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all