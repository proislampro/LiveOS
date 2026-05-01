# Path to your posix-uefi directory
UEFI_PATH       := uefi
ARCH            := x86_64
TARGET          := BOOTX64.EFI
BUILD_DIR       := build

CC              := gcc
LD              := ld
OBJCOPY         := objcopy

SRC             := main.c
OBJ             := $(BUILD_DIR)/main.o
SO_FILE         := $(BUILD_DIR)/bootloader.so
EFI             := $(BUILD_DIR)/$(TARGET)


CFLAGS := -ffreestanding -fno-stack-protector -fno-stack-check \
          -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
          -fpic -Wno-main -I$(UEFI_PATH)


LDFLAGS := -nostdlib -shared -Bsymbolic \
           -L$(UEFI_PATH) \
           -T $(UEFI_PATH)/elf_$(ARCH)_efi.lds


OBJCOPY_FLAGS := -j .text -j .sdata -j .data -j .dynamic \
                 -j .dynsym -j .rel -j .rela -j .reloc \
                 --target=efi-app-$(ARCH)

all: $(EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(EFI): $(OBJ)
	@$(LD) $(LDFLAGS) $(UEFI_PATH)/crt_$(ARCH).o \
		$(OBJ) -o $(SO_FILE) -luefi
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $(SO_FILE) $(EFI)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all