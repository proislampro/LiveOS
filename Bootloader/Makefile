# Path to your posix-uefi directory
# Update this to where you cloned/installed posix-uefi
UEFI_PATH       := ~/posix-uefi

ARCH            := x86_64
TARGET          := BOOTX64.EFI
BUILD_DIR       := build

# POSIX-UEFI provides a helper include to set up all flags
include $(UEFI_PATH)/Makefile.conf

# Overriding/Setting paths for POSIX-UEFI headers and libraries
CFLAGS          += -I$(UEFI_PATH)/inc -I$(UEFI_PATH)/inc/$(ARCH)
LDFLAGS         += -L$(UEFI_PATH)/$(ARCH)/lib

SRC             := main.c
OBJ             := $(BUILD_DIR)/main.o
# POSIX-UEFI usually links directly to the EFI binary using its own rules
EFI             := $(BUILD_DIR)/$(TARGET)

all: $(EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EFI): $(OBJ)
	# POSIX-UEFI uses a specific linker script and crt0 provided in its repo
	$(LD) $(LDFLAGS) $(UEFI_PATH)/$(ARCH)/gnuefi/crt0-efi-$(ARCH).o \
		$^ -o $(BUILD_DIR)/bootloader.so -luefi $(LOADLIBES)
	# Convert to EFI
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .reloc --target=efi-app-$(ARCH) \
		$(BUILD_DIR)/bootloader.so $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all
