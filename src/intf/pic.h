#pragma once

void pic_remap();
void pic_send_eoi(unsigned char irq);
void pic_unmask(uint8_t irq);
