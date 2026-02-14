#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

/* Multiboot2 tag types */
#define MULTIBOOT_TAG_TYPE_END          0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER  8

/* Generic tag structure */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

/* Framebuffer tag structure */
struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t reserved;
};

#endif