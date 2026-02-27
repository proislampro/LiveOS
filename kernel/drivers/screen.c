#include <stdint.h>
#define VGA_WIDTH 80
#define VGA_HEIGHT 22
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT)
#define MAX_PRINT_LOG 100

static volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
static uint16_t vga_index = VGA_WIDTH;
static uint8_t cursor_color = 0x0F;
static char cursor = '_';
static char default_color = 0x0F;
static uint16_t saved_cell = 0x0F20;
static uint16_t print_log[MAX_PRINT_LOG];
static uint16_t print_log_pointer = 0;
static char app_title[80] = "LiveOS";

char getdefault_color() { return default_color; }
void setdefault_color(char color) { default_color = color; }
int getpointer() { return vga_index; }
char* get_app_title() { return app_title; }
void set_app_title(char* title) {
    int i = 0;
    while (i < 79 && title[i] != '\0') {
        app_title[i] = title[i];
        i++;
    }
    app_title[i] = '\0';
}
void hide_cursor() {
    vga_buffer[vga_index] = saved_cell;
}
void show_cursor() {
    saved_cell = vga_buffer[vga_index];
    vga_buffer[vga_index] = ((uint16_t)cursor_color << 8) | (uint8_t)cursor;
}
void refresh_cursor() { show_cursor(); }
void changcursor(char cha) { hide_cursor(); cursor = cha; show_cursor(); }
void changcursor_color(char color) { hide_cursor(); cursor_color = color; show_cursor(); }
void fix_cursor() { outb(0x3D4, 0x0A); outb(0x3D5, 0x20); show_cursor(); }
void movpointer_to(int pos) {
    if (pos < VGA_WIDTH || pos >= VGA_SIZE - VGA_WIDTH) return;
    hide_cursor();
    vga_index = (uint16_t)pos;
    show_cursor();
}
void movpointer(int offset) {
    hide_cursor();
    int new_pos = (int)vga_index + offset;
    if (new_pos < VGA_WIDTH) vga_index = VGA_WIDTH;
    else if (new_pos >= VGA_SIZE - VGA_WIDTH) vga_index = VGA_SIZE - VGA_WIDTH - 1;
    else vga_index = (uint16_t)new_pos;
    show_cursor();
}
void cleanscreen(char character, char color) {
    uint16_t blank = ((uint16_t)color << 8) | (uint8_t)character;
    for (int i = VGA_WIDTH; i < VGA_SIZE - VGA_WIDTH; i++) vga_buffer[i] = blank;
    vga_index = VGA_WIDTH;
    print_log_pointer = 0;
    saved_cell = blank;
    show_cursor();
}
void scroll(int lines) {
    if (lines <= 0) return;
    hide_cursor();
    if (lines >= VGA_HEIGHT - 2) {
        cleanscreen(' ', default_color);
        return;
    }
    int move_limit = (VGA_HEIGHT - 2 - lines) * VGA_WIDTH;
    for (int i = 0; i < move_limit; i++) vga_buffer[i + VGA_WIDTH] = vga_buffer[i + (lines + 1) * VGA_WIDTH];
    uint16_t blank = ((uint16_t)default_color << 8) | ' ';
    for (int i = move_limit + VGA_WIDTH; i < VGA_SIZE - VGA_WIDTH; i++) vga_buffer[i] = blank;
    int scroll_offset = lines * VGA_WIDTH;
    int new_pointer = 0;
    for (int i = 0; i < print_log_pointer; i++) {
        if (print_log[i] >= scroll_offset) print_log[new_pointer++] = print_log[i] - scroll_offset;
    }
    print_log_pointer = new_pointer;
    vga_index = (VGA_HEIGHT - 2) * VGA_WIDTH;
    show_cursor();
}
void printchar_in(char c, uint8_t color, uint16_t position) {
    if (position >= VGA_WIDTH && position < VGA_SIZE - VGA_WIDTH) vga_buffer[position] = ((uint16_t)color << 8) | (uint8_t)c;
}
static void printchar_raw(char c, uint8_t color) {
    if (c == '\n') {
        if (print_log_pointer < MAX_PRINT_LOG) print_log[print_log_pointer++] = vga_index;
        vga_index = ((vga_index / VGA_WIDTH) + 1) * VGA_WIDTH;
        if (vga_index >= VGA_SIZE - VGA_WIDTH) scroll(1);
    }
    else if (c == '\b') {
        if (vga_index == VGA_WIDTH) return;
        if (print_log_pointer > 0 && vga_index % 80 == 0) {
            vga_index = print_log[print_log_pointer-1];
            print_log_pointer--;
            return;
        }
        vga_index--;
        printchar_in(' ', color, vga_index);
    }
    else if (c == '\t') {
        uint16_t col = vga_index % VGA_WIDTH;
        uint16_t spaces = 8 - (col % 8);
        vga_index += spaces;
    }
    else printchar_in(c, color, vga_index++);
    while (vga_index >= VGA_SIZE - VGA_WIDTH) { scroll(1); vga_index -= VGA_WIDTH; }
}
void printchar(char c, uint8_t color) { hide_cursor(); printchar_raw(c, color); show_cursor(); }
uint8_t hex_to_int(char c) { if (c >= '0' && c <= '9') return c - '0'; c |= 0x20; if (c >= 'a' && c <= 'f') return c - 'a' + 10; return 0; }
uint32_t str_to_hex(char* str) { uint32_t val = 0; while (*str) val = (val << 4) | hex_to_int(*str++); return val; }
void print_string(char* string) {
    uint8_t current_color = getdefault_color();
    hide_cursor();
    for (uint32_t i = 0; string[i] != '\0'; i++) {
        if (string[i] == '&' && string[i+1] != '\0' && string[i+2] != '\0') {
            char bg = string[i+1]; char fg = string[i+2];
            uint8_t new_bg = (bg == 'n') ? (default_color >> 4) : hex_to_int(bg);
            uint8_t new_fg = (fg == 'n') ? (default_color & 0x0F) : hex_to_int(fg);
            current_color = (new_bg << 4) | new_fg; i += 2; continue;
        }
        printchar_raw(string[i], current_color);
    }
    show_cursor();
}
void print_string_inversed(char* string) {
    uint8_t current_color = ((uint8_t)getdefault_color() >> 4) | ((uint8_t)getdefault_color() << 4);
    hide_cursor();
    for (uint32_t i = 0; string[i] != '\0'; i++) printchar_raw(string[i], current_color);
    show_cursor();
}
void update_app_title() {
    int title_length = 0;
    while (app_title[title_length] != '\0' && title_length < 80) title_length++;
    int start = (80 - title_length) / 2;
    for (int i = 0; i < 80; i++) vga_buffer[i] = (uint16_t)(' ' | (0x1F << 8));
    for (int i = 0; i < title_length; i++) vga_buffer[start + i] = (uint16_t)(app_title[i] | (0x1F << 8));
}
void init_screen() { cleanscreen(' ', default_color); fix_cursor(); update_app_title(); }
