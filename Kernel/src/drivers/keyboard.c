#include <stdint.h>

const char scancode_normal[128] = {
    0,  27, '&','e','"','\'','(','-','e','_','c','a',')','=','\b',
    '\t','a','z','e','r','t','y','u','i','o','p','^','$','\n',
    0,   'q','s','d','f','g','h','j','k','l','m','u','*',0,
    '\\','w','x','c','v','b','n',',',';',':','!',0, '*',0, ' ',
};

const char scancode_shift[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','*','+','\b',
    '\t','A','Z','E','R','T','Y','U','I','O','P','^','`','\n',
    0,   'Q','S','D','F','G','H','J','K','L','M','%','*',0,
    '|', 'W','X','C','V','B','N','?','.','/','&',0, '*',0, ' ',
};

static int shift_active = 0;
static int caps_lock = 0;


char get_key() {
    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);
            
            // Handle shift press
            if (scancode == 0x2A || scancode == 0x36) {
                shift_active = 1;
                continue;
            }
            // Handle shift release
            if (scancode == 0xAA || scancode == 0xB6) {
                shift_active = 0;
                continue;
            }
            // Handle caps lock toggle
            if (scancode == 0x3A) {
                caps_lock = !caps_lock;
                continue;
            }
            
            // Ignore key releases and invalid scancodes
            if (scancode & 0x80 || scancode >= 128) continue;

            // FIXED: Caps lock only affects letters, shift affects everything
            char c;
            if (shift_active || caps_lock) {
                // Shift is pressed - use shift table
                c = scancode_shift[scancode];
            } else {
                // Shift not pressed - use normal table
                c = scancode_normal[scancode];
            }

            if (c > 0) return c;
        }
    }
}

void scan_string(char* buffer, int max_size, char end_btn) {
    int i = 0;
    while (1) {
        char c = get_key();
        
        if (c == end_btn) break;

        if (c == '\b') {
            if (i > 0) {
                i--;
                // printchar('\b', getdefault_color());
            }
        } else if ((c >= ' ' || c == '\n') && i < max_size - 1) {
            buffer[i++] = c;
            // printchar(c, getdefault_color());
        }
    }
    buffer[i] = '\0';
    // printchar('\n', getdefault_color());
}