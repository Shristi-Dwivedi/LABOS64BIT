#include "framebuffer.h"
#include <stdint.h>
#include <stdbool.h>
#include "multiboot2.h"

int framebuffer_available(void) {  
    return screen.addr != 0;
}
void parse_multiboot2(uint64_t addr) {
    struct multiboot_tag *tag;
    for(tag = (struct multiboot_tag*)(addr + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {

        if(tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            struct multiboot_tag_framebuffer *fb_tag = (void*)tag;

            // Initialize screen struct
            screen.addr   = (uint32_t*)fb_tag->addr;
            screen.width  = fb_tag->width;
            screen.height = fb_tag->height;
            screen.pitch  = fb_tag->pitch;
            screen.bpp    = fb_tag->bpp;

            // Initialize framebuffer (optional)
            fb_init(fb_tag->addr, fb_tag->width, fb_tag->height, fb_tag->pitch, fb_tag->bpp);
        }
    }
}