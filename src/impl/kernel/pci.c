#include "pci.h"
#include "io.h"
#include "storage.h"
#include "console.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// storage_device_t g_storage = {0};

static uint32_t pci_addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    return (1u << 31)
         | ((uint32_t)bus  << 16)
         | ((uint32_t)slot << 11)
         | ((uint32_t)func << 8)
         | (offset & 0xFC);
}

uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    outl(PCI_CONFIG_ADDR, pci_addr(bus, slot, func, offset));
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t v = pci_read32(bus, slot, func, offset);
    return (uint16_t)((v >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t v = pci_read32(bus, slot, func, offset);
    return (uint8_t)((v >> ((offset & 3) * 8)) & 0xFF);
}

static void print_hex8(uint8_t v)
{
    const char *hex = "0123456789ABCDEF";
    char s[3];
    s[0] = hex[(v >> 4) & 0xF];
    s[1] = hex[v & 0xF];
    s[2] = 0;
    console_write(s);
}

static void print_hex16(uint16_t v)
{
    print_hex8((uint8_t)(v >> 8));
    print_hex8((uint8_t)(v & 0xFF));
}

static void print_hex32(uint32_t v)
{
    print_hex16((uint16_t)(v >> 16));
    print_hex16((uint16_t)(v & 0xFFFF));
}

void pci_scan_storage(void)
{
    console_write("Scanning PCI storage controllers...\n");

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = pci_read16(bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue;

                uint8_t class_code = pci_read8(bus, slot, func, 0x0B);
                uint8_t subclass   = pci_read8(bus, slot, func, 0x0A);
                uint8_t prog_if    = pci_read8(bus, slot, func, 0x09);

                // Mass storage class
                if (class_code != 0x01) continue;

                // AHCI SATA
                if (subclass == 0x06 && prog_if == 0x01) {
                    uint32_t bar5 = pci_read32(bus, slot, func, 0x24);

                    console_write("AHCI found: vendor=");
                    print_hex16(vendor);
                    console_write(" device=");
                    print_hex16(pci_read16(bus, slot, func, 0x02));
                    console_write(" BAR5=");
                    print_hex32(bar5);
                    console_write("\n");

                    g_storage.type = STORAGE_AHCI;
                    g_storage.mmio_base = (uint64_t)(bar5 & ~0xF);
                    g_storage.vendor_id = vendor;
                    g_storage.device_id = pci_read16(bus, slot, func, 0x02);
                    g_storage.bus = (uint8_t)bus;
                    g_storage.slot = slot;
                    g_storage.func = func;
                    return;
                }

                // NVMe
                if (subclass == 0x08 && prog_if == 0x02) {
                    uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                    uint32_t bar1 = pci_read32(bus, slot, func, 0x14);
                    uint64_t mmio = ((uint64_t)bar1 << 32) | (bar0 & ~0xF);

                    console_write("NVMe found: vendor=");
                    print_hex16(vendor);
                    console_write(" device=");
                    print_hex16(pci_read16(bus, slot, func, 0x02));
                    console_write(" BAR0=");
                    print_hex32(bar0);
                    console_write("\n");

                    g_storage.type = STORAGE_NVME;
                    g_storage.mmio_base = mmio;
                    g_storage.vendor_id = vendor;
                    g_storage.device_id = pci_read16(bus, slot, func, 0x02);
                    g_storage.bus = (uint8_t)bus;
                    g_storage.slot = slot;
                    g_storage.func = func;
                    return;
                }
            }
        }
    }

    console_write("No AHCI/NVMe controller found.\n");
}