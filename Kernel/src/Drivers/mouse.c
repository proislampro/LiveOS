#include <stdint.h>
#include <stdbool.h>
#include <inout.h>
#include <screen.h>
#include <kprintf.c>
#include <mouse.h>

static uint8_t mouse_cycle        = 0;
static uint8_t mouse_packet[MOUSE_PACKET_SIZE];
static int     mouse_position[2]  = { MOUSE_DEFAULT_X, MOUSE_DEFAULT_Y };

void move_mouse_cursor(int x_move, int y_move) {
    mouse_position[0] += x_move;
    mouse_position[1] += y_move;
    serial_printf("Mouse moved to: (%d, %d)\n", mouse_position[0], mouse_position[1]);
}

void handle_mouse_packet(uint8_t *packet) {
    bool left   = packet[0] & MOUSE_BTN_LEFT;
    bool right  = packet[0] & MOUSE_BTN_RIGHT;
    bool middle = packet[0] & MOUSE_BTN_MIDDLE;

    int x_move = (int)packet[1];
    int y_move = (int)packet[2];

    if (packet[0] & MOUSE_X_SIGN)
        x_move |= ~0xFF;
    if (packet[0] & MOUSE_Y_SIGN)
        y_move |= ~0xFF;

    y_move = -y_move;

    serial_printf(
        "Mouse packet: b0=0x%x b1=0x%x b2=0x%x buttons[L:%d M:%d R:%d]\n",
        packet[0], packet[1], packet[2],
        left, middle, right
    );

    move_mouse_cursor(x_move, y_move);
}

void mouse_handler(void) {
    uint8_t status = inb(PS2_STATUS_PORT);

    if ((status & PS2_STATUS_OUT_FULL) && (status & PS2_STATUS_MOUSE_DATA)) {
        uint8_t data = inb(PS2_DATA_PORT);

        switch (mouse_cycle) {
            case 0:
                if (!(data & MOUSE_ALWAYS_ONE)) return;
                mouse_packet[0] = data;
                mouse_cycle++;
                break;
            case 1:
                mouse_packet[1] = data;
                mouse_cycle++;
                break;
            case 2:
                mouse_packet[2] = data;
                handle_mouse_packet(mouse_packet);
                mouse_cycle = 0;
                break;
        }
    }

    outb(PIC_SLAVE_CMD,  PIC_EOI);
    outb(PIC_MASTER_CMD, PIC_EOI);
}

void mouse_dispatcher(char *ps2_data) {
    (void)ps2_data;
    mouse_handler();
}