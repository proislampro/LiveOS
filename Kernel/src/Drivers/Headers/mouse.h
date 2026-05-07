#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

/* ── PS/2 Controller I/O Ports ─────────────────────────────────────────── */
#define PS2_DATA_PORT           0x60        /* Read/write PS/2 data          */
#define PS2_STATUS_PORT         0x64        /* Read controller status        */
#define PS2_CMD_PORT            0x64        /* Write controller commands     */

/* ── PS/2 Status Register Bits ────────────────────────────────────────── */
#define PS2_STATUS_OUT_FULL     (1 << 0)    /* Output buffer has data        */
#define PS2_STATUS_IN_FULL      (1 << 1)    /* Input buffer is full          */
#define PS2_STATUS_MOUSE_DATA   (1 << 5)    /* Data in buffer is from mouse  */

/* ── PIC EOI (End-of-Interrupt) ────────────────────────────────────────── */
#define PIC_MASTER_CMD          0x20        /* Master PIC command port       */
#define PIC_SLAVE_CMD           0xA0        /* Slave  PIC command port       */
#define PIC_EOI                 0x20        /* End-of-interrupt signal       */

/* ── Mouse Packet Layout ───────────────────────────────────────────────── */
#define MOUSE_PACKET_SIZE       3           /* Standard PS/2 packet bytes    */

/* Byte 0 button bits */
#define MOUSE_BTN_LEFT          (1 << 0)
#define MOUSE_BTN_RIGHT         (1 << 1)
#define MOUSE_BTN_MIDDLE        (1 << 2)
#define MOUSE_ALWAYS_ONE        (1 << 3)    /* Must be 1 — sync check        */
#define MOUSE_X_SIGN            (1 << 4)    /* X delta sign bit              */
#define MOUSE_Y_SIGN            (1 << 5)    /* Y delta sign bit              */
#define MOUSE_X_OVERFLOW        (1 << 6)    /* X delta overflowed            */
#define MOUSE_Y_OVERFLOW        (1 << 7)    /* Y delta overflowed            */

/* ── Default Cursor Position ───────────────────────────────────────────── */
#define MOUSE_DEFAULT_X         400
#define MOUSE_DEFAULT_Y         300

/* ── Function Prototypes ───────────────────────────────────────────────── */
void mouse_handler(void);
void mouse_dispatcher(char *ps2_data);
void handle_mouse_packet(uint8_t *packet);
void move_mouse_cursor(int x_move, int y_move);

#endif /* MOUSE_H */