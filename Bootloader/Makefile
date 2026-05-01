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

# CFLAGS: 
# -I$(UEFI_PATH) points to where uefi.h lives
CFLAGS := -ffreestanding -fno-stack-protector -fno-stack-check \
          -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
          -fpic -Wno-main -I$(UEFI_PATH)

# LDFLAGS:
# -L$(UEFI_PATH) points to where libuefi.a lives
# -T points to the linker script in your listing
LDFLAGS := -nostdlib -shared -Bsymbolic \
           -L$(UEFI_PATH) \
           -T $(UEFI_PATH)/elf_$(ARCH)_efi.lds

# OBJCOPY:
# Converts the ELF shared object into a PE/COFF UEFI binary
OBJCOPY_FLAGS := -j .text -j .sdata -j .data -j .dynamic \
                 -j .dynsym -j .rel -j .rela -j .reloc \
                 --target=efi-app-$(ARCH)

all: $(EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(EFI): $(OBJ)
	# 1. Link the object files with crt_x86_64.o and libuefi.a
	# Note: Use your existing .o file or compile the .c if needed
	@$(LD) $(LDFLAGS) $(UEFI_PATH)/crt_$(ARCH).o \
		$(OBJ) -o $(SO_FILE) -luefi
	# 2. Convert the resulting .so to .efi
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $(SO_FILE) $(EFI)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all