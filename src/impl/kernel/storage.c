#include <stdint.h>
#include "storage.h"
#include "pci.h"
#include "console.h"
#include "ahci.h"

storage_device_t g_storage = {0};

static int ahci_init(void);
static int ahci_read_sector(uint64_t lba, uint8_t *buffer);
static int ahci_write_sector(uint64_t lba, const uint8_t *buffer);

static int nvme_init(void);
static int nvme_read_sector(uint64_t lba, uint8_t *buffer);
static int nvme_write_sector(uint64_t lba, const uint8_t *buffer);

static hba_mem_t *abar = 0;
static hba_port_t *port = 0;

/* one-port simple static memory for AHCI structures
   works well for early kernel/QEMU experiments */
static uint8_t ahci_cmd_list[1024] __attribute__((aligned(1024)));
static uint8_t ahci_fis[256]       __attribute__((aligned(256)));
static uint8_t ahci_cmd_table[256] __attribute__((aligned(128)));

static void memzero(void *ptr, uint32_t size)
{
    uint8_t *p = (uint8_t *)ptr;
    for (uint32_t i = 0; i < size; i++) p[i] = 0;
}

static void memcpy8(void *dst, const void *src, uint32_t size)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < size; i++) d[i] = s[i];
}

static void stop_cmd(hba_port_t *p)
{
    p->cmd &= ~HBA_PxCMD_ST;
    p->cmd &= ~HBA_PxCMD_FRE;

    while (p->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR)) {
    }
}

static void start_cmd(hba_port_t *p)
{
    while (p->cmd & HBA_PxCMD_CR) {
    }

    p->cmd |= HBA_PxCMD_FRE;
    p->cmd |= HBA_PxCMD_ST;
}

int storage_init(void)
{
    pci_scan_storage();

    if (g_storage.type == STORAGE_AHCI) {
        console_write("Initializing AHCI...\n");
        return ahci_init();
    }

    if (g_storage.type == STORAGE_NVME) {
        console_write("Initializing NVMe...\n");
        return nvme_init();
    }

    return -1;
}

int storage_read_sector(uint64_t lba, uint8_t *buffer)
{
    if (g_storage.type == STORAGE_AHCI)
        return ahci_read_sector(lba, buffer);

    if (g_storage.type == STORAGE_NVME)
        return nvme_read_sector(lba, buffer);

    return -1;
}

int storage_write_sector(uint64_t lba, const uint8_t *buffer)
{
    if (g_storage.type == STORAGE_AHCI)
        return ahci_write_sector(lba, buffer);

    if (g_storage.type == STORAGE_NVME)
        return nvme_write_sector(lba, buffer);

    return -1;
}

static int ahci_init(void)
{
    abar = (hba_mem_t *)(uintptr_t)g_storage.mmio_base;

    if (!abar) {
        console_write("AHCI BAR invalid\n");
        return -1;
    }

    console_write("AHCI Base OK\n");

    uint32_t pi = abar->pi;
    if (pi == 0) {
        console_write("No AHCI ports\n");
        return -1;
    }

    console_write("Ports:\n");

    for (int i = 0; i < 32; i++) {
        if (pi & (1u << i)) {
            console_write("Port found\n");
            port = (hba_port_t *)((uint8_t *)abar + 0x100 + (i * 0x80));
            break;
        }
    }

    if (!port) return -1;

    stop_cmd(port);

    memzero(ahci_cmd_list, sizeof(ahci_cmd_list));
    memzero(ahci_fis, sizeof(ahci_fis));
    memzero(ahci_cmd_table, sizeof(ahci_cmd_table));

    port->clb  = (uint32_t)(uintptr_t)ahci_cmd_list;
    port->clbu = 0;
    port->fb   = (uint32_t)(uintptr_t)ahci_fis;
    port->fbu  = 0;

    start_cmd(port);

    return 0;
}

static int ahci_read_sector(uint64_t lba, uint8_t *buffer)
{
    if (!port) return -1;

    /* wait until port not busy */
    int spin = 1000000;
    while ((port->tfd & (0x80 | 0x08)) && spin > 0) {
        spin--;
    }
    if (spin == 0) return -2;

    port->is = 0xFFFFFFFF;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)ahci_cmd_list;
    memzero(cmdheader, sizeof(hba_cmd_header_t) * 32);

    cmdheader[0].cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmdheader[0].w = 0;      /* read */
    cmdheader[0].prdtl = 1;
    cmdheader[0].ctba = (uint32_t)(uintptr_t)ahci_cmd_table;
    cmdheader[0].ctbau = 0;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t *)ahci_cmd_table;
    memzero(cmdtbl, sizeof(hba_cmd_tbl_t));

    cmdtbl->prdt_entry[0].dba = (uint32_t)(uintptr_t)buffer;
    cmdtbl->prdt_entry[0].dbau = 0;
    cmdtbl->prdt_entry[0].dbc = 512 - 1; /* one sector */
    cmdtbl->prdt_entry[0].i = 1;

    fis_reg_h2d_t *cfis = (fis_reg_h2d_t *)(&cmdtbl->cfis[0]);
    memzero(cfis, sizeof(fis_reg_h2d_t));

    cfis->fis_type = 0x27; /* Register H2D */
    cfis->c = 1;           /* command */
    cfis->command = ATA_CMD_READ_DMA_EX;
    cfis->device = 1 << 6; /* LBA mode */

    cfis->lba0 = (uint8_t)(lba & 0xFF);
    cfis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
    cfis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
    cfis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
    cfis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
    cfis->lba5 = (uint8_t)((lba >> 40) & 0xFF);

    cfis->countl = 1; /* one sector */
    cfis->counth = 0;

    /* issue command in slot 0 */
    port->ci = 1;

    spin = 1000000;
    while ((port->ci & 1) && spin > 0) {
        spin--;
    }

    if (spin == 0) return -3;
    if (port->is & (1 << 30)) return -4; /* task file error */

    return 0;
}

static int ahci_write_sector(uint64_t lba, const uint8_t *buffer)
{
    if (!port) return -1;

    // wait until port not busy
    int spin = 1000000;
    while ((port->tfd & (0x80 | 0x08)) && spin > 0) {
        spin--;
    }
    if (spin == 0) return -2;

    port->is = 0xFFFFFFFF;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)ahci_cmd_list;
    memzero(cmdheader, sizeof(hba_cmd_header_t) * 32);

    cmdheader[0].cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmdheader[0].w = 1;      // write
    cmdheader[0].prdtl = 1;
    cmdheader[0].ctba = (uint32_t)(uintptr_t)ahci_cmd_table;
    cmdheader[0].ctbau = 0;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t *)ahci_cmd_table;
    memzero(cmdtbl, sizeof(hba_cmd_tbl_t));

    cmdtbl->prdt_entry[0].dba = (uint32_t)(uintptr_t)buffer;
    cmdtbl->prdt_entry[0].dbau = 0;
    cmdtbl->prdt_entry[0].dbc = 512 - 1;   // one sector
    cmdtbl->prdt_entry[0].i = 1;

    fis_reg_h2d_t *cfis = (fis_reg_h2d_t *)(&cmdtbl->cfis[0]);
    memzero(cfis, sizeof(fis_reg_h2d_t));

    cfis->fis_type = 0x27;   // Register H2D
    cfis->c = 1;             // command
    cfis->command = ATA_CMD_WRITE_DMA_EX;
    cfis->device = 1 << 6;   // LBA mode

    cfis->lba0 = (uint8_t)(lba & 0xFF);
    cfis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
    cfis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
    cfis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
    cfis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
    cfis->lba5 = (uint8_t)((lba >> 40) & 0xFF);

    cfis->countl = 1;        // one sector
    cfis->counth = 0;

    // issue command in slot 0
    port->ci = 1;

    spin = 1000000;
    while ((port->ci & 1) && spin > 0) {
        spin--;
    }

    if (spin == 0) return -3;
    if (port->is & (1 << 30)) return -4;   // task file error

    return 0;
}

static int nvme_init(void)
{
    return -3;
}

static int nvme_read_sector(uint64_t lba, uint8_t *buffer)
{
    (void)lba;
    (void)buffer;
    return -3;
}

static int nvme_write_sector(uint64_t lba, const uint8_t *buffer)
{
    (void)lba;
    (void)buffer;
    return -3;
}