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
    uint8_t framebuffer_info[6];
};

static uint8_t *framebuffer_addr = 0;
static uint32_t framebuffer_pitch = 0;
static uint32_t framebuffer_width = 0;
static uint32_t framebuffer_height = 0;
static uint8_t framebuffer_bpp = 0;
static uint8_t framebuffer_type = 0;
static uint8_t red_field_position = 16;
static uint8_t red_mask_size = 8;
static uint8_t green_field_position = 8;
static uint8_t green_mask_size = 8;
static uint8_t blue_field_position = 0;
static uint8_t blue_mask_size = 8;

static uint32_t scale_channel(uint8_t value, uint8_t bits) {
    if (bits == 0) {
        return 0;
    }

    uint32_t max_out = (1u << bits) - 1u;
    return ((uint32_t)value * max_out + 127u) / 255u;
}

static uint32_t encode_color(uint32_t color) {
    uint8_t red = (uint8_t)((color >> 16) & 0xFF);
    uint8_t green = (uint8_t)((color >> 8) & 0xFF);
    uint8_t blue = (uint8_t)(color & 0xFF);

    if (framebuffer_type != 1) {
        return color;
    }

    uint32_t packed = 0;
    packed |= scale_channel(red, red_mask_size) << red_field_position;
    packed |= scale_channel(green, green_mask_size) << green_field_position;
    packed |= scale_channel(blue, blue_mask_size) << blue_field_position;
    return packed;
}

void init_screen(uint64_t multiboot_info) {
    if (!multiboot_info) return;

    uint8_t *tag = (uint8_t *)multiboot_info + 8;

    while (1) {
        struct multiboot_tag_header *t = (struct multiboot_tag_header *)tag;

        if (t->size < sizeof(struct multiboot_tag_header)) {
            break;
        }

        if (t->type == 8) {
            struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer *)tag;

            if (fb->addr == 0) break;

            framebuffer_addr = (uint8_t *)(uint64_t)fb->addr;
            framebuffer_pitch = fb->pitch;
            framebuffer_width = fb->width;
            framebuffer_height = fb->height;
            framebuffer_bpp = fb->bpp;
            framebuffer_type = fb->framebuffer_type;

            if (fb->framebuffer_type == 1) {
                red_field_position = fb->framebuffer_info[0];
                red_mask_size = fb->framebuffer_info[1];
                green_field_position = fb->framebuffer_info[2];
                green_mask_size = fb->framebuffer_info[3];
                blue_field_position = fb->framebuffer_info[4];
                blue_mask_size = fb->framebuffer_info[5];
            }

            return;
        }

        if (t->type == 0) {
            break;
        }

        tag += (t->size + 7) & ~7;
    }

    framebuffer_addr = 0;
    framebuffer_pitch = framebuffer_width = framebuffer_height = framebuffer_bpp = 0;
    framebuffer_type = 0;
}

void putpixel(int x, int y, uint32_t color) {
    if (!framebuffer_addr || (framebuffer_bpp != 24 && framebuffer_bpp != 32)) {
        return;
    }

    if (x < 0 || y < 0 || (uint32_t)x >= framebuffer_width || (uint32_t)y >= framebuffer_height) {
        return;
    }

    uint8_t *pixel = framebuffer_addr + (uint32_t)y * framebuffer_pitch + (uint32_t)x * (framebuffer_bpp / 8u);
    uint32_t encoded = encode_color(color);

    pixel[0] = (uint8_t)(encoded & 0xFF);
    pixel[1] = (uint8_t)((encoded >> 8) & 0xFF);
    pixel[2] = (uint8_t)((encoded >> 16) & 0xFF);

    if (framebuffer_bpp == 32) {
        pixel[3] = (uint8_t)((encoded >> 24) & 0xFF);
    }
}

void draw_rect(int start_x, int start_y, int width, int height, uint32_t color) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            putpixel(start_x + x, start_y + y, color);
        }
    }
}
