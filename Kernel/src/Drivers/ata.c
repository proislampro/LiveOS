#include <stdint.h>
#include <stddef.h>
#include <ata.h>
#include <serial.h>
#include <inout.h>

uint16_t bus_ports[] = { 0x1F0, 0x170 };

void ata_init(void) {
    if (ata_identify(0, 0) == ATA_SUCCESS) {
        serial_printf("Master drive detected.\n");
    } else {
        serial_printf("No master drive found.\n");
    }
    ata_wait_ready(0); // Wait for master drive to be ready
}

int ata_identify(uint8_t channel, uint8_t device) {
    outb(bus_ports[channel] + ATA_DRIVE_SEL, (device == 0) ? 0xA0 : 0xB0); // Select drive
    outb(bus_ports[channel] + ATA_SECTOR_CNT, 0); // Clear sector count
    outb(bus_ports[channel] + ATA_LBA_LOW, 0);    // Clear LBA low
    outb(bus_ports[channel] + ATA_LBA_MID, 0);    // Clear LBA mid
    outb(bus_ports[channel] + ATA_LBA_HIGH, 0);   // Clear LBA high
    outb(bus_ports[channel] + ATA_COMMAND, ATA_CMD_IDENTIFY); // Send IDENTIFY command

    uint8_t status = inb(bus_ports[channel] + ATA_STATUS);
    if (status == 0) {
        return ATA_ERROR_RETURN; // No device
    }

    while ((status & ATA_STAT_BSY) || !(status & ATA_STAT_DRQ)) {
        status = inb(bus_ports[channel] + ATA_STATUS);
        if (status & ATA_STAT_ERR) {
            return ATA_ERROR_RETURN; // Error occurred
        }
    }

    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(bus_ports[channel] + ATA_DATA);
    }

    return ATA_SUCCESS;
}

int ata_read_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer) {
    outb(bus_ports[channel] + ATA_DRIVE_SEL, (device == 0) ? 0xE0 : (0xF0 | ((lba >> 24) & 0x0F))); // Select drive and set LBA bits
    outb(bus_ports[channel] + ATA_SECTOR_CNT, count); // Set sector count
    outb(bus_ports[channel] + ATA_LBA_LOW, lba & 0xFF); // Set LBA low
    outb(bus_ports[channel] + ATA_LBA_MID, (lba >> 8) & 0xFF); // Set LBA mid
    outb(bus_ports[channel] + ATA_LBA_HIGH, (lba >> 16) & 0xFF); // Set LBA high
    outb(bus_ports[channel] + ATA_COMMAND, ATA_CMD_READ); // Send READ command

    uint8_t *byte_buffer = (uint8_t *)buffer;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t status = inb(bus_ports[channel] + ATA_STATUS);
        while ((status & ATA_STAT_BSY) || !(status & ATA_STAT_DRQ)) {
            status = inb(bus_ports[channel] + ATA_STATUS);
            if (status & ATA_STAT_ERR) {
                return ATA_ERROR_RETURN; // Error occurred
            }
        }

        for (int j = 0; j < 256; j++) {
            uint16_t data = inw(bus_ports[channel] + ATA_DATA);
            byte_buffer[i * 512 + j * 2] = data & 0xFF;
            byte_buffer[i * 512 + j * 2 + 1] = (data >> 8) & 0xFF;
        }
    }

    return ATA_SUCCESS;
}
int ata_write_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer) {
    outb(bus_ports[channel] + ATA_DRIVE_SEL, (device == 0) ? 0xE0 : (0xF0 | ((lba >> 24) & 0x0F))); // Select drive and set LBA bits
    outb(bus_ports[channel] + ATA_SECTOR_CNT, count); // Set sector count
    outb(bus_ports[channel] + ATA_LBA_LOW, lba & 0xFF); // Set LBA low
    outb(bus_ports[channel] + ATA_LBA_MID, (lba >> 8) & 0xFF); // Set LBA mid
    outb(bus_ports[channel] + ATA_LBA_HIGH, (lba >> 16) & 0xFF); // Set LBA high
    outb(bus_ports[channel] + ATA_COMMAND, ATA_CMD_WRITE); // Send WRITE command

    uint8_t *byte_buffer = (uint8_t *)buffer;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t status = inb(bus_ports[channel] + ATA_STATUS);
        while ((status & ATA_STAT_BSY) || !(status & ATA_STAT_DRQ)) {
            status = inb(bus_ports[channel] + ATA_STATUS);
            if (status & ATA_STAT_ERR) {
                return ATA_ERROR_RETURN; // Error occurred
            }
        }

        for (int j = 0; j < 256; j++) {
            uint16_t data = byte_buffer[i * 512 + j * 2] | (byte_buffer[i * 512 + j * 2 + 1] << 8);
            outw(bus_ports[channel] + ATA_DATA, data);
        }
    }

    return ATA_SUCCESS;
}
uint8_t ata_read_status(uint8_t channel) {
    return inb(bus_ports[channel] + ATA_STATUS);
}
void ata_wait_ready(uint8_t channel) {
    while (inb(bus_ports[channel] + ATA_STATUS) & ATA_STAT_BSY);
}