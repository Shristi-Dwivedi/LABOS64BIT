#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

typedef enum{
    STORAGE_NONE = 0,
    STORAGE_AHCI = 1,
    STORAGE_NVME = 2
} storage_type_t;

typedef struct {
    storage_type_t type;
    uint64_t mmio_base;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
} storage_device_t;

extern storage_device_t g_storage;

int storage_init(void);
int storage_read_sector(uint64_t lba, uint8_t *buffer);
int storage_write_sector(uint64_t lba, const uint8_t *buffer);

#endif