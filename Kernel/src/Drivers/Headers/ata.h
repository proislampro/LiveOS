#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stddef.h>

/* ATA Command Codes */
#define ATA_CMD_READ_SECTORS        0x20
#define ATA_CMD_WRITE_SECTORS       0x30
#define ATA_CMD_IDENTIFY_DEVICE     0xEC
#define ATA_CMD_SET_FEATURES        0xEF

/* ATA Status Register Bits */
#define ATA_STATUS_BUSY             0x80
#define ATA_STATUS_READY            0x40
#define ATA_STATUS_FAULT            0x20
#define ATA_STATUS_SEEK_COMPLETE    0x10
#define ATA_STATUS_DATA_REQUEST     0x08
#define ATA_STATUS_CORRECTED_DATA   0x04
#define ATA_STATUS_INDEX            0x02
#define ATA_STATUS_ERROR            0x01

/* ATA Error Register Bits */
#define ATA_ERROR_BAD_BLOCK         0x80
#define ATA_ERROR_UNCORRECTABLE     0x40
#define ATA_ERROR_MEDIA_CHANGED     0x20
#define ATA_ERROR_ID_NOT_FOUND      0x10
#define ATA_ERROR_MEDIA_CHANGE_REQ  0x08
#define ATA_ERROR_ABORT             0x04
#define ATA_ERROR_TRACK_0_NOT_FOUND 0x02
#define ATA_ERROR_ADDRESS_MARK      0x01

/* ATA Port Offsets */
#define ATA_REG_DATA                0x00
#define ATA_REG_ERROR               0x01
#define ATA_REG_FEATURES            0x01
#define ATA_REG_SECTOR_COUNT        0x02
#define ATA_REG_LBA_LOW             0x03
#define ATA_REG_LBA_MID             0x04
#define ATA_REG_LBA_HIGH            0x05
#define ATA_REG_DEVICE              0x06
#define ATA_REG_COMMAND             0x07
#define ATA_REG_STATUS              0x07
#define ATA_REG_CONTROL             0x206
#define ATA_REG_ALT_STATUS          0x206

/* Device Selection */
#define ATA_DEVICE_MASTER           0x00
#define ATA_DEVICE_SLAVE            0x10

// Error codes
#define ATA_SUCCESS     0
#define ATA_ERROR       1
#define ATA_TIMEOUT    -1

/* ATA Device Structure */
typedef struct {
    uint8_t channel;
    uint8_t device;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t command_sets;
    uint32_t size;
    char serial[21];
    char model[41];
    char firmware[9];
} ATA_Device;

/* ATA Channel Structure */
typedef struct {
    uint16_t base;
    uint16_t ctrl;
    uint8_t drive_count;
    ATA_Device devices[2];
} ATA_Channel;

/* Function Declarations */
void ata_init(void);
int ata_identify(uint8_t channel, uint8_t device);
int ata_read_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
int ata_write_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
uint8_t ata_read_status(uint8_t channel);
void ata_wait_ready(uint8_t channel);

#endif /* ATA_H */
