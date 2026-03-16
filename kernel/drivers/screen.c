#include <stdint.h>


struct multiboot_tag_header {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;

    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
};

static uint8_t *framebuffer_addr = 0;
static uint32_t framebuffer_pitch = 0;
static uint32_t framebuffer_width = 0;
static uint32_t framebuffer_height = 0;
static uint8_t framebuffer_bpp = 0;

void init_screen(uint64_t multiboot_info) {

    if (!multiboot_info) {
        framebuffer_addr = (uint8_t *)0xE0000000;
        framebuffer_pitch = 1024 * 4;
        framebuffer_width = 1024;
        framebuffer_height = 768;
        framebuffer_bpp = 32;
        return;
    }

    uint8_t *tag = (uint8_t *)multiboot_info + 8;

    while (1) {
        struct multiboot_tag_header *t = (struct multiboot_tag_header *)tag;

        if (t->type == 0) break;

        if (t->type == 8) {
            struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer *)tag;

            framebuffer_addr  = (uint8_t *)(uint64_t)fb->addr;
            framebuffer_pitch = fb->pitch;
            framebuffer_width = fb->width;
            framebuffer_height = fb->height;
            framebuffer_bpp = fb->bpp;
            return;
        }

        tag += (t->size + 7) & ~7;  // align to 8 bytes
    }
}

void putpixel(int x, int y, uint32_t color) {
    if (!framebuffer_addr || framebuffer_bpp != 32) {
        return;
    }

    if (x < 0 || y < 0 || (uint32_t)x >= framebuffer_width || (uint32_t)y >= framebuffer_height) {
        return;
    }

    uint32_t *pixel = (uint32_t *)(framebuffer_addr + (uint32_t)y * framebuffer_pitch + (uint32_t)x * 4);
    *pixel = color;
}

void draw_rect(int start_x, int start_y, int width, int height, uint32_t color) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            putpixel(start_x + x, start_y + y, color);
        }
    }
}
