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
SHELLO     = shell.o
SHELLB	   = shell.app

CC = gcc
AS = nasm
LD = ld

CFLAGS  = -ffreestanding -m64 -O2 -Wall -Wextra -fno-pic -nostdlib
LDFLAGS = -T linker.ld -nostdlib
APP_LDFLAGS = -T apps.ld -nostdlib

all: clean $(IMAGE_NAME)

$(KERNEL): $(KERNELO) $(BOOTO)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(BOOTO) $(KERNELO)

$(KERNELO): $(KERNELC)
	$(CC) $(CFLAGS) -c $(KERNELC) -o $(KERNELO)

$(BOOTO): $(KERNELE)
	$(AS) -f elf64 $(KERNELE) -o $(BOOTO)

$(SHELLB): $(SHELLC)
	$(CC) $(CFLAGS) -c $(SHELLC) -o $(APPS_DIR)/$(SHELLO)
	$(LD) $(APP_LDFLAGS) -o $(APPS_DIR)/$(SHELLB) $(APPS_DIR)/$(SHELLO)





# =========================
# Disk Image (BIOS + MBR)
# =========================
$(IMAGE_NAME): $(KERNEL) $(CONFIG) $(SHELLB)
	# Create empty 64MB disk
	truncate -s 64M $(IMAGE_NAME)

	# Create GPT + EFI System Partition
	parted -s $(IMAGE_NAME) \
		mklabel gpt \
		mkpart ESP fat32 1MiB 100% \
		set 1 esp on

	# Format partition as FAT32
	mformat -i $(IMAGE_NAME)@@1M -F

	# Create EFI boot path (required by UEFI spec)
	mmd -i $(IMAGE_NAME)@@1M ::/EFI
	mmd -i $(IMAGE_NAME)@@1M ::/EFI/BOOT

	# Copy Limine EFI loader to the standard UEFI path
	mcopy -i $(IMAGE_NAME)@@1M \
		$(LIMINE_DIR)/BOOTX64.EFI \
		::/EFI/BOOT/

	# Copy kernel and config (Limine looks for these at the root or /boot/)
	mmd -i $(IMAGE_NAME)@@1M ::/boot
	mcopy -i $(IMAGE_NAME)@@1M \
		$(KERNEL) \
		$(CONFIG) \
		$(LIMINE_DIR)/limine-uefi-cd.bin \
		::/boot/

	# Apps
	mmd -i $(IMAGE_NAME)@@1M ::/apps
	mcopy -i $(IMAGE_NAME)@@1M -s $(APPS_DIR)/$(SHELLB) ::/apps/
	mcopy -i $(IMAGE_NAME)@@1M -s hello.txt ::/

	# No limine install step needed for UEFI — the EFI binary is self-sufficient

run:
	qemu-system-x86_64 \
		-bios /usr/share/ovmf/OVMF.fd \
		-drive format=raw,file=$(IMAGE_NAME) \
		-m 512M

clean:
	rm -f binairies/*.o $(KERNEL) $(IMAGE_NAME)

.PHONY: all run clean
