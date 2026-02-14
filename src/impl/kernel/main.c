#include "print.h"
#include "idt.h"
#include "keyboard.h"
#include "io.h"
#include "pic.h"
#include "console.h"

void kernel_main(uint16_t mb_addr){
    vga_clear();
    vga_print("Welcome to LABOS\n");
    vga_print("Start Typing\n");
    idt_init();
    pic_remap();
    pic_unmask_irq(0);
    pic_unmask_irq(1);
    keyboard_init();

    asm volatile("sti");    // enable interrupts
    while(1){
        asm volatile("hlt");
    }
}
