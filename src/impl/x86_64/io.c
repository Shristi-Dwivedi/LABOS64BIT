#include "io.h"
#include <stdint.h>

// input

uint8_t inb(uint16_t port){
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

uint16_t inw(uint16_t port){
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

uint32_t inl(uint16_t port){
    uint32_t result;
    __asm__ volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
}

// output

void outb(uint16_t port, uint8_t value){
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void outw(uint16_t port, uint16_t value){
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void outl(uint16_t port, uint32_t value){
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}