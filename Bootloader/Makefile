# Tools
CC      = gcc
LD      = ld
OBJCOPY = objcopy

# Paths
EFI_INCLUDE = /usr/include/efi
EFI_LIB     = /usr/lib

# Files
SRC = main.c
OBJ = build/main.o
SO  = build/main.so
EFI = build/BOOTX64.EFI
 

CFLAGS = -I$(EFI_INCLUDE) -I$(EFI_INCLUDE)/x86_64 \
         -fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
         -DGNU_EFI_USE_MS_ABI -Wall -Wextra -c \
         -g -Og

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_LIB)/elf_x86_64_efi.lds \
          -shared -Bsymbolic $(EFI_LIB)/crt0-efi-x86_64.o \
          -L$(EFI_LIB) -lefi -lgnuefi

OBJCOPY_FLAGS = -j .text -j .sdata -j .data \
                -j .dynamic -j .dynsym -j .rel \
                -j .rela -j .reloc \
                --target=efi-app-x86_64 --subsystem=10


all: $(EFI)

$(OBJ): $(SRC)
	@$(CC) $(CFLAGS) $< -o $@

$(SO): $(OBJ)
	@$(LD) $(LDFLAGS) $(OBJ) -o $@

$(EFI): $(SO)
	@$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@
	@rm -f $(SO) $(OBJ)


clean:
	@rm -rf $(OBJ) $(SO) $(EFI)

.PHONY: all clean