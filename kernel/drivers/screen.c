#include <stdint.h>

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
};

uint32_t* framebuffer;
uint32_t width;
uint32_t height;
uint32_t pitch;


void init_screen(uint64_t multiboot_info)
{
    uint8_t *tag = (uint8_t *)multiboot_info + 8;

    while (1)
    {
        struct multiboot_tag *t = (struct multiboot_tag*)tag;

        if (t->type == 8) // framebuffer tag
        {
            struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer*)tag;

            return;
        }

        if (t->type == 0) // end tag
            break;

        tag += (t->size + 7) & ~7;
    }

    while (1) {} // framebuffer not found
}

void putpixel(int x, int y, uint32_t color)
{
    uint32_t* pixel;

    pixel = (uint32_t*)((uint8_t*)framebuffer + y * pitch + x * 4);

    *pixel = color;
}