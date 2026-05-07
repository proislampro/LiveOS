#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stddef.h>

/* ── I/O Port Offsets (Add these to the Bus Base) ──────────────────────── */
#define ATA_DATA         0   /* Data register (R/W)                     */
#define ATA_ERROR        1   /* Error register (R)                      */
#define ATA_FEATURES     1   /* Features register (W)                   */
#define ATA_SECTOR_CNT   2   /* Sector count register                   */
#define ATA_LBA_LOW      3   /* LBA low byte                            */
#define ATA_LBA_MID      4   /* LBA mid byte                            */
#define ATA_LBA_HIGH     5   /* LBA high byte                           */
#define ATA_DRIVE_SEL    6   /* Drive/head select register              */
#define ATA_STATUS       7   /* Status register (R)                     */
#define ATA_COMMAND      7   /* Command register (W)                    */

/* Note: Control registers are usually on a different base (0x3F6) 
   and don't follow the 0-7 offset pattern. Keeping them absolute for now. */
#define ATA_ALT_STATUS   0x3F6   
#define ATA_DEV_CTRL     0x3F6   

/* ── ATA Commands ──────────────────────────────────────────────────────── */
#define ATA_CMD_READ         0x20    
#define ATA_CMD_WRITE        0x30    
#define ATA_CMD_IDENTIFY     0xEC    
#define ATA_CMD_FLUSH        0xE7    

/* ── Status Register Flags ─────────────────────────────────────────────── */
#define ATA_STAT_ERR     (1 << 0)    
#define ATA_STAT_DRQ     (1 << 3)    
#define ATA_STAT_SRV     (1 << 4)    
#define ATA_STAT_DF      (1 << 5)    
#define ATA_STAT_RDY     (1 << 6)    
#define ATA_STAT_BSY     (1 << 7)    

/* ── Drive Select Masks ────────────────────────────────────────────────── */
#define ATA_DRIVE_MASTER_LBA     0xE0    
#define ATA_DRIVE_SLAVE_LBA      0xF0    

/* ── Return Codes ──────────────────────────────────────────────────────── */
#define ATA_SUCCESS       0
#define ATA_ERROR_RETURN -1

/* ── Geometry / Limits ─────────────────────────────────────────────────── */
#define ATA_SECTOR_SIZE     512      
#define ATA_IDENTIFY_WORDS  256      

/* ── Function Prototypes ───────────────────────────────────────────────── */
void    ata_init(void);
int     ata_identify(uint8_t channel, uint8_t device);
int     ata_read_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
int     ata_write_sectors(uint8_t channel, uint8_t device, uint32_t lba, uint32_t count, void *buffer);
uint8_t ata_read_status(uint8_t channel);
void    ata_wait_ready(uint8_t channel);

#endif /* ATA_H */