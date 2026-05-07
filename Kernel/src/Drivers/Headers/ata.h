#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stddef.h>

/* ── I/O Ports ─────────────────────────────────────────────────────────── */
#define ATA_DATA        0x1F0   /* Data register (R/W)                      */
#define ATA_ERROR       0x1F1   /* Error register (R)                        */
#define ATA_FEATURES    0x1F1   /* Features register (W)                     */
#define ATA_SECTOR_CNT  0x1F2   /* Sector count register                     */
#define ATA_LBA_LOW     0x1F3   /* LBA low byte                              */
#define ATA_LBA_MID     0x1F4   /* LBA mid byte                              */
#define ATA_LBA_HIGH    0x1F5   /* LBA high byte                             */
#define ATA_DRIVE_SEL   0x1F6   /* Drive/head select register                */
#define ATA_STATUS      0x1F7   /* Status register (R)                       */
#define ATA_COMMAND     0x1F7   /* Command register (W)                      */
#define ATA_ALT_STATUS  0x3F6   /* Alternate status register (R)             */
#define ATA_DEV_CTRL    0x3F6   /* Device control register (W)               */

/* ── ATA Commands ──────────────────────────────────────────────────────── */
#define ATA_CMD_READ        0x20    /* Read sectors (with retry)             */
#define ATA_CMD_WRITE       0x30    /* Write sectors (with retry)            */
#define ATA_CMD_IDENTIFY    0xEC    /* Identify drive                        */
#define ATA_CMD_FLUSH       0xE7    /* Flush cache                           */
#define ATA_CMD_READ_DMA    0xC8    /* Read DMA                              */
#define ATA_CMD_WRITE_DMA   0xCA    /* Write DMA                             */

/* ── Status Register Flags ─────────────────────────────────────────────── */
#define ATA_STAT_ERR    (1 << 0)    /* Error occurred                        */
#define ATA_STAT_IDX    (1 << 1)    /* Index (always 0)                      */
#define ATA_STAT_CORR   (1 << 2)    /* Corrected data (always 0)             */
#define ATA_STAT_DRQ    (1 << 3)    /* Data request — ready for PIO transfer */
#define ATA_STAT_SRV    (1 << 4)    /* Overlapped mode service request       */
#define ATA_STAT_DF     (1 << 5)    /* Drive fault (does not set ERR)        */
#define ATA_STAT_RDY    (1 << 6)    /* Drive ready                           */
#define ATA_STAT_BSY    (1 << 7)    /* Drive busy                            */

/* ── Drive Select Masks ────────────────────────────────────────────────── */
#define ATA_DRIVE_MASTER_CHS    0xA0    /* Select master, CHS mode           */
#define ATA_DRIVE_SLAVE_CHS     0xB0    /* Select slave,  CHS mode           */
#define ATA_DRIVE_MASTER_LBA    0xE0    /* Select master, LBA mode           */
#define ATA_DRIVE_SLAVE_LBA     0xF0    /* Select slave,  LBA mode           */
#define ATA_DRIVE_LBA28_MASK    0x0F    /* LBA bits 24-27 mask               */

/* ── Return Codes ──────────────────────────────────────────────────────── */
#define ATA_SUCCESS      0
#define ATA_ERROR       -1

/* ── Geometry / Limits ─────────────────────────────────────────────────── */
#define ATA_SECTOR_SIZE     512         /* Bytes per sector                  */
#define ATA_IDENTIFY_WORDS  256         /* Words returned by IDENTIFY        */

/* ── Function Prototypes ───────────────────────────────────────────────── */
void    ata_init(void);
int     ata_identify(uint8_t channel, uint8_t device);
int     ata_read_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
int     ata_write_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
uint8_t ata_read_status(uint8_t channel);
void    ata_wait_ready(uint8_t channel);

#endif /* ATA_H */