
IMAGE_NAME  := disk.img
KERNEL      := Kernel/build/kernel.elf
MY_BOOT     := Bootloader/build/BOOTX64.EFI

LIMINE_DIR  := ./Limine
CONFIG      := $(LIMINE_DIR)/limine.conf

QEMU        := qemu-system-x86_64
OVMF        := /usr/share/ovmf/OVMF.fd

MTOOLS_IMG  := -i $(IMAGE_NAME)@@1M


.PHONY: all multiboot singleboot run clean kernel bootloader


all:
	@$(MAKE) multiboot --no-print-directory

kernel:
	@echo "@ Building    : Kernel"
	@$(MAKE) -C Kernel --no-print-directory

bootloader:
	@echo "@ Building    : Bootloader"
	@$(MAKE) -C Bootloader --no-print-directory



$(IMAGE_NAME): kernel bootloader
	@echo "@ Building    : $(IMAGE_NAME)"
	@dd if=/dev/zero of=$(IMAGE_NAME) bs=1M count=64 status=none
	@parted -s $(IMAGE_NAME) mklabel gpt
	@parted -s $(IMAGE_NAME) mkpart ESP fat32 2048s 100%
	@parted -s $(IMAGE_NAME) set 1 esp on
	@mformat $(MTOOLS_IMG) -F ::
	@mmd     $(MTOOLS_IMG) ::/boot ::/EFI ::/EFI/BOOT
	@mmd     $(MTOOLS_IMG) ::/system
	@mmd     $(MTOOLS_IMG) ::/system/bin
	@mcopy   $(MTOOLS_IMG) $(KERNEL) ::/system/bin/livekernel.elf
	@mcopy   $(MTOOLS_IMG) hello.txt ::/hello.txt

multiboot: $(IMAGE_NAME)
	@echo "@ Installing  : Limine + LiveOS Bootloader"
	-@mdel  $(MTOOLS_IMG) ::/boot/limine.conf        2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(CONFIG)                  ::/boot/limine.conf
	-@mdel  $(MTOOLS_IMG) ::/EFI/BOOT/BOOTX64.EFI   2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(LIMINE_DIR)/BOOTX64.EFI  ::/EFI/BOOT/BOOTX64.EFI
	@mmd    $(MTOOLS_IMG) ::/EFI/LiveOS
	-@mdel  $(MTOOLS_IMG) ::/EFI/LiveOS/BOOTX64.EFI  2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(MY_BOOT)                 ::/EFI/LiveOS/BOOTX64.EFI

singleboot: $(IMAGE_NAME)
	@echo "@ Installing  : LiveOS bootloader"
	-@mdel  $(MTOOLS_IMG) ::/EFI/BOOT/BOOTX64.EFI   2>/dev/null; true
	@mcopy  $(MTOOLS_IMG) $(MY_BOOT)                 ::/EFI/BOOT/BOOTX64.EFI


MEMORY := 512

run:
	@echo "@ Qemu VM     :"
	@echo "  $$  BIOS       -->   OVMF-UEFI ($(OVMF))"
	@echo "  $$  ROM        -->   $(IMAGE_NAME)"
	@echo "  $$  RAM        -->   $(MEMORY) MB"
	@$(QEMU) \
	    -bios  $(OVMF) \
	    -drive format=raw,file=$(IMAGE_NAME) \
	    -m     $(MEMORY)M \
	    -net   none \
	    -s

clean:
	@echo "@ Cleaning    :"
	@$(MAKE) -C Kernel     clean --no-print-directory
	@$(MAKE) -C Bootloader clean --no-print-directory
	@rm -f $(IMAGE_NAME) 