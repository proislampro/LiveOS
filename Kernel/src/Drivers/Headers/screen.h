#ifndef SCREEN_H
#define SCREEN_H


typedef struct {
    uint64_t addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
} framebuffer_t;


void init_screen();
void put_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_container(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t border_color,uint32_t border_width, uint32_t fill_color);

#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 16

void print_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void print_string(const char* str, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);

#endif /* SCREEN_H */