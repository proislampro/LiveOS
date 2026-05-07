#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <Font.c>
#include <screen.h>
#include <serial.h>

framebuffer_t fb;

// 1. Tell Limine we want a framebuffer
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};


void init_screen() {


    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        while(1);
    }

    // 3. Get the first available framebuffer
    struct limine_framebuffer* limine_fb = framebuffer_request.response->framebuffers[0];

    fb.addr   = (uint64_t)limine_fb->address;
    fb.width  = limine_fb->width;
    fb.height = limine_fb->height;
    fb.pitch  = limine_fb->pitch;
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    // Note: Use fb.pitch instead of width*4 because pitch includes padding
    uint32_t* pixel_addr = (uint32_t*)(fb.addr + y * fb.pitch + x * 4);
    *pixel_addr = color;
}

void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            put_pixel(x + i, y + j, color);
        }
    }
}

void draw_container(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t border_color,uint32_t border_width, uint32_t fill_color) {
    draw_rect(x, y, width, height, fill_color); // Fill
    draw_rect(x, y, width, border_width, border_color); // Top border
    draw_rect(x, y + height - border_width, width, border_width, border_color); // Bottom border
    draw_rect(x, y, border_width, height, border_color); // Left border
    draw_rect(x + width - border_width, y, border_width, height, border_color); // Right border
}


void print_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    for (uint32_t j = 0; j < FONT_CHAR_HEIGHT; j++) {
        for (uint32_t i = 0; i < FONT_CHAR_WIDTH; i++) {
            if (font8x16[(uint8_t)c][j] & (1 << (7 - i))) {
                put_pixel(x + i, y + j, fg_color);
            } else {
                put_pixel(x + i, y + j, bg_color);
            }
        }
    }
    serial_printf("%c", c);
}

void print_string(const char* str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    uint32_t orig_x = x;
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            y += FONT_CHAR_HEIGHT;
            x = orig_x;
        } else {
            print_char(str[i], x, y, fg_color, bg_color);
            x += FONT_CHAR_WIDTH;
        }
    }
}