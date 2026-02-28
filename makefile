IMAGE_NAME = disk.img
KERNEL     = binairies/kernel.elf
KERNELE    = kernel/kernel_entry.s
KERNELC    = kernel/kernel.c
BOOTO      = binairies/boot.o
KERNELO    = binairies/kernel.o
LIMINE_DIR = limine
CONFIG     = limine.conf

APPS_DIR   = binairies/apps
SHELLC     = apps/shell.c
SHELLB	   = shell.app

CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld

CFLAGS  = -ffreestanding -m32 -O2 -Wall -Wextra -fno-pic -nostdlib
LDFLAGS = -T linker.ld -nostdlib

all: clean $(IMAGE_NAME)

$(KERNEL): $(KERNELO) $(BOOTO)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(BOOTO) $(KERNELO)

$(KERNELO): $(KERNELC)
	$(CC) $(CFLAGS) -c $(KERNELC) -o $(KERNELO)

$(BOOTO): $(KERNELE)
	$(AS) -f elf32 $(KERNELE) -o $(BOOTO)

$(SHELLB): $(SHELLC)
	$(CC) $(CFLAGS) $(SHELLC) -o $(APPS_DIR)/$(SHELLB)




# =========================
# Disk Image (BIOS + MBR)
# =========================
$(IMAGE_NAME): $(KERNEL) $(CONFIG) $(SHELLB)
	# Create empty 64MB disk
	truncate -s 64M $(IMAGE_NAME)

	# Create MBR + FAT32 partition
	parted -s $(IMAGE_NAME) \
		mklabel msdos \
		mkpart primary fat32 1MiB 100% \
		set 1 boot on

	# Format partition
	mformat -i $(IMAGE_NAME)@@1M -F

	# Create /boot directory
	mmd -i $(IMAGE_NAME)@@1M ::/boot

	# Copy required files
	mcopy -i $(IMAGE_NAME)@@1M \
		$(KERNEL) \
		$(CONFIG) \
		$(LIMINE_DIR)/limine-bios.sys \
		::/boot/
	
	mmd -i $(IMAGE_NAME)@@1M ::/apps

	mcopy -i $(IMAGE_NAME)@@1M -s $(APPS_DIR)/$(SHELLB) ::/apps/
	mcopy -i $(IMAGE_NAME)@@1M -s hello.txt ::/

	# Install Limine BIOS
	$(LIMINE_DIR)/limine bios-install $(IMAGE_NAME)


# =========================
# Utilities
# =========================
run:
	qemu-system-i386 \
	-drive format=raw,file=$(IMAGE_NAME) \
	-m 512M

clean:
	rm -f binairies/*.o $(KERNEL) $(IMAGE_NAME)

.PHONY: all run clean