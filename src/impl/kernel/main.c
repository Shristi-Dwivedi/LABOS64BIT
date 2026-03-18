#include <stdint.h>
#include "multiboot2.h"
#include "framebuffer.h"
#include "console.h"
#include "pic.h"
#include "keyboard.h"
#include "shell.h"
#include "cursor.h"
#include "mouse.h"
#include "gui.h"

extern void init_idt();
extern void pic_remap();
extern void pic_unmask(uint8_t irq);

void kernel_main(uint64_t magic, uint64_t addr)
{
    parse_multiboot2(addr); // framebuffer filled here
    fb_clear();
    pic_remap();
    init_idt();
    mouse_init();
    pic_unmask(0);
    pic_unmask(1); // Unmask keyboard IRQ
    pic_unmask(12);
    __asm__ volatile("sti"); // enable interrupts
    console_putc('\n');
    // shell_init();
    gui_enter();
    while (1)
    {
        cursor_draw();
        __asm__ volatile("hlt");
    }
}
