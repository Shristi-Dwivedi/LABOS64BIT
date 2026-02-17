#include <stdint.h>
#include "multiboot2.h"
#include "framebuffer.h"
#include "console.h"
#include "pic.h"
#include "keyboard.h"

extern void init_idt();
extern void pic_remap();
extern void pic_unmask(uint8_t irq);

void kernel_main(uint64_t magic, uint64_t addr)
{
    fb_clear();
    parse_multiboot2(addr); // framebuffer filled here
    console_init();
    // console_write("LABOS started! Press Enter to start typing ...\n");
    // console_putc('A');
    pic_remap();
    init_idt();
    pic_unmask(0);
    pic_unmask(1);
    __asm__ volatile("sti"); // enable interrupts

    while (1)
    {
        __asm__ volatile("hlt");
    }
}
