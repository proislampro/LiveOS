
IMAGE_NAME  := disk.img
KERNEL      := Kernel/build/kernel.elf
MY_BOOT     := Bootloader/build/BOOTX64.EFI

LIMINE_DIR  := ./Limine
CONFIG      := $(LIMINE_DIR)/limine.conf

QEMU        := qemu-system-x86_64
OVMF        := /usr/share/ovmf/OVMF.fd

MTOOLS_IMG  := -i $(IMAGE_NAME)@@1M


.PHONY: all multiboot singleboot run clean kernel bootloader


all: $(IMAGE_NAME) multiboot

kernel:
	@echo "  BUILD  kernel"
	@$(MAKE) -C Kernel --no-print-directory

bootloader:
	@echo "  BUILD  bootloader"
	@$(MAKE) -C Bootloader --no-print-directory



$(IMAGE_NAME): kernel bootloader
	@echo "  IMG    $(IMAGE_NAME)"
	@dd if=/dev/zero of=$(IMAGE_NAME) bs=1M count=64 status=none
	@parted -s $(IMAGE_NAME) mklabel gpt
	@parted -s $(IMAGE_NAME) mkpart ESP fat32 2048s 100%
	@parted -s $(IMAGE_NAME) set 1 esp on
	@mformat $(MTOOLS_IMG) -F ::
	@mmd     $(MTOOLS_IMG) ::/boot ::/EFI ::/EFI/BOOT
	@mcopy   $(MTOOLS_IMG) $(KERNEL) ::/boot/kernel.elf

multiboot: $(IMAGE_NAME)
	@echo "  BOOT   multiboot (Limine + LiveOS)"
	-@mdel  $(MTOOLS_IMG) ::/boot/limine.conf        2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(CONFIG)                  ::/boot/limine.conf
	-@mdel  $(MTOOLS_IMG) ::/EFI/BOOT/BOOTX64.EFI   2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(LIMINE_DIR)/BOOTX64.EFI  ::/EFI/BOOT/BOOTX64.EFI
	@mmd    $(MTOOLS_IMG) ::/EFI/LiveOS
	-@mdel  $(MTOOLS_IMG) ::/EFI/LiveOS/BOOTX64.EFI  2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(MY_BOOT)                 ::/EFI/LiveOS/BOOTX64.EFI

singleboot: $(IMAGE_NAME)
	@echo "  BOOT   singleboot (LiveOS only)"
	-@mdel  $(MTOOLS_IMG) ::/EFI/BOOT/BOOTX64.EFI   2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(MY_BOOT)                 ::/EFI/BOOT/BOOTX64.EFI


run:
	@echo "  QEMU   $(IMAGE_NAME)"
	@$(QEMU) \
	    -bios  $(OVMF)              \
	    -drive format=raw,file=$(IMAGE_NAME) \
	    -m     512M                 \
	    -net   none                 \
	    -s

clean:
	@echo "  CLEAN"
	@$(MAKE) -C Kernel     clean --no-print-directory
	@$(MAKE) -C Bootloader clean --no-print-directory
	@rm -f $(IMAGE_NAME)