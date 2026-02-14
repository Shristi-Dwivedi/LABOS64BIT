#include <stdint.h>
#include "multiboot2.h"

int framebuffer_available = 0;

uint64_t fb_addr;
uint32_t fb_width;
uint32_t fb_height;
uint32_t fb_pitch;

void framebuffer_detect(uint64_t mb_addr)
{
    uint8_t* addr = (uint8_t*)mb_addr;
    addr += 8;

    while (1)
    {
        struct multiboot_tag* tag = (struct multiboot_tag*)addr;

        if (tag->type == MULTIBOOT_TAG_TYPE_END)
            break;

        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
        {
            struct multiboot_tag_framebuffer* fb =
                (struct multiboot_tag_framebuffer*)tag;

            framebuffer_available = 1;
            fb_addr = fb->framebuffer_addr;
            fb_width = fb->framebuffer_width;
            fb_height = fb->framebuffer_height;
            fb_pitch = fb->framebuffer_pitch;
        }

        addr += (tag->size + 7) & ~7;
    }
}
