#include <stdint.h>
#include <stddef.h>

// ATA Register Ports
#define ATA_DATA        0x1F0
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SEL   0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

// Status Bits
#define ATA_STAT_ERR    0x01
#define ATA_STAT_DRQ    0x08
#define ATA_STAT_BSY    0x80

// Commands
#define ATA_CMD_READ    0x20
#define ATA_CMD_WRITE   0x30
#define ATA_CMD_FLUSH   0xE7

// Error codes
#define ATA_SUCCESS     0
#define ATA_ERROR       1
#define ATA_TIMEOUT    -1

void ata_io_wait() {
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

void ata_wait_bsy() {
    int timeout = 1000000;  // Add timeout
    while ((inb(ATA_STATUS) & ATA_STAT_BSY) && timeout-- > 0);
}

int ata_wait_drq() {
    int timeout = 100000;
    
    while (timeout-- > 0) {
        uint8_t status = inb(ATA_STATUS);
        
        if (status & ATA_STAT_ERR) {
            return ATA_ERROR;
        }
        
        if (status & ATA_STAT_DRQ) {
            return ATA_SUCCESS;
        }
        
        ata_io_wait();
    }
    
    return ATA_TIMEOUT;
}

static void ata_prepare(uint32_t lba, uint8_t count) {
    ata_wait_bsy();
    
    outb(ATA_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    outb(ATA_SECTOR_CNT, count);
    
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
}



int read_sector(uint32_t lba, uint8_t* buffer) {
    if (!buffer) return ATA_ERROR;
    
    ata_prepare(lba, 1);
    outb(ATA_COMMAND, ATA_CMD_READ);
    ata_io_wait();

    int drq_status = ata_wait_drq();
    if (drq_status != ATA_SUCCESS) return drq_status;

    insw(ATA_DATA, buffer, 256);
    
    if (inb(ATA_STATUS) & ATA_STAT_ERR) return ATA_ERROR;
    
    return ATA_SUCCESS; 
}


int write_sector(uint32_t lba, uint8_t* buffer) {
    if (!buffer) return ATA_ERROR;
    
    ata_prepare(lba, 1);
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    ata_io_wait();

    int drq_status = ata_wait_drq();
    if (drq_status != ATA_SUCCESS) {
        return drq_status;
    }

    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA, ptr[i]);
    }

    outb(ATA_COMMAND, ATA_CMD_FLUSH);
    ata_wait_bsy();
    
    uint8_t status = inb(ATA_STATUS);
    if (status & ATA_STAT_ERR) {
        return ATA_ERROR;
    }
    
    return ATA_SUCCESS;
}

int block_read(uint64_t lba, void *buf, size_t count) {
    uint8_t *byte_buf = (uint8_t *)buf;
    for (size_t i = 0; i < count; ++i) {
        int res = read_sector((uint32_t)(lba + i), byte_buf + (i * 512));
        if (res != ATA_SUCCESS) {
            return res;
        }
    }
    return ATA_SUCCESS;
}

int block_write(uint64_t lba, const void *buf, size_t count) {
    const uint8_t *byte_buf = (const uint8_t *)buf;
    for (size_t i = 0; i < count; ++i) {
        int res = write_sector((uint32_t)(lba + i), (uint8_t *)(byte_buf + (i * 512)));
        if (res != ATA_SUCCESS) {
            return res;
        }
    }
    return ATA_SUCCESS;
}