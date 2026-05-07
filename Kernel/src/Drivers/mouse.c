#include <stdint.h>
#include <stdbool.h>

uint8_t mouse_cycle = 0;
uint8_t mouse_packet[3];

int mouse_position[2] = {400, 300};

void mouse_handler(void) {
    uint8_t status = inb(0x64);

    // Check if data is actually available and if it's from the mouse
    if ((status & 0x01) && (status & 0x20)) {
        uint8_t data = inb(0x60);

        // State machine to collect the 3-byte packet
        switch(mouse_cycle) {
            case 0:
                // Bit 3 of the first byte should always be 1
                if (!(data & 0x08)) return; // Discard out-of-sync data
                mouse_packet[0] = data;
                mouse_cycle++;
                break;
            case 1:
                mouse_packet[1] = data;
                mouse_cycle++;
                break;
            case 2:
                mouse_packet[2] = data;
                handle_mouse_packet(mouse_packet); // Process the full packet
                mouse_cycle = 0; // Reset for next packet
                break;
        }
    }

    outb(0xA0, 0x20); // Slave PIC
    outb(0x20, 0x20); // Master PIC
}

void move_mouse_cursor(int x_move, int y_move) {
    mouse_position[0] += x_move;
    mouse_position[1] += y_move;
    serial_printf("Mouse moved to: (%d, %d)\n", mouse_position[0], mouse_position[1]);
}

void handle_mouse_packet(uint8_t* packet) {
    bool left   = packet[0] & 0x01;
    bool right  = packet[0] & 0x02;
    bool middle = packet[0] & 0x04;

    // 1. Start with the raw 8-bit values
    int x_move = (int)packet[1];
    int y_move = (int)packet[2];

    // 2. Sign-extend to 32 bits if needed
    if (packet[0] & 0x10) { // X Sign bit
        x_move |= ~0xFF;
    }
    if (packet[0] & 0x20) { // Y Sign bit
        y_move |= ~0xFF;
    }

    y_move = -y_move;

    serial_printf(
        "Mouse packet: b0=0x%x b1=0x%x b2=0x%x buttons[L:%d M:%d R:%d]\n",
        packet[0],
        packet[1],
        packet[2],
        left,
        middle,
        right
    );

    move_mouse_cursor(x_move, y_move);
}

void mouse_dispatcher(char* ps2_data) {
    (void)ps2_data;
    mouse_handler();
}
