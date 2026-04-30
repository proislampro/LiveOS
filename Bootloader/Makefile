.RECIPEPREFIX = >

#___________ configs _____________

CC      = x86_64-w64-mingw32-gcc
LD      = x86_64-w64-mingw32-ld

EFI_LIB_DIR = /usr/include/efi
EFI_LIB     = /usr/lib

BUILD_DIR   ?= build

BOOT_SRC    ?= main.c

#_________ compiler _________


CFLAGS = -fshort-wchar\
        -mno-red-zone\
		-ffreestanding\
		-fno-stack-protector\
		-Wall\
		-Wextra\
		-I$(EFI_LIB_DIR)\
		-I$(EFI_LIB)

LDFLAGS = \
	--nostdlib\
	--entry=uefi_main\
	--subsystem 10


#____ main ____

SRC_CFILES  = $(EFI_LIB_SRC)  $(BOOT_SRC) 

OBJECTS     = $(BUILD_DIR)/main.o

BOOT_OUTPUT = $(BUILD_DIR)/BOOTX64.EFI

#_____________ RECEPIES ____________

# bootloader
$(BOOT_OUTPUT) : $(OBJECTS) | $(BUILD_DIR)

> @$(LD) $(LDFLAGS) -o $@ $^

# objects
$(BUILD_DIR)/%.o : %.c

> @mkdir -p $(dir $@)
> @$(CC) $(CFLAGS) -c $< -o $@


#___________ PHONY ____________

.PHONY : all clean

all_target = $(BOOT_OUTPUT)

all    : $(all_target)

clean  :
> @rm -f $(OBJECTS)
> @rm -f $(BOOT_OUTPUT)