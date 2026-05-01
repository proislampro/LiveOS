
typedef struct {
    uint64_t addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} framebuffer;

framebuffer fb;

void init_screen(uint64_t fb_addr, uint32_t fb_width, uint32_t fb_height, uint32_t fb_pitch) {
    fb.addr = fb_addr;
    fb.width = fb_width;
    fb.height = fb_height;
    fb.pitch = fb_pitch;
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
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