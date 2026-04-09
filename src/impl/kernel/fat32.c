#include <stdint.h>
#include "fat32.h"
#include "storage.h"
#include "console.h"

static FAT32_BootSector bs;

static uint32_t fat_start_lba = 0;
static uint32_t data_start_lba = 0;
static uint32_t root_cluster = 0;

// Prototype of functions
static void fat32_print_name11(const uint8_t name[11]);
static void fat32_make_83_name(const char *src, uint8_t out[11]);

static void print_hex8(uint8_t v)
{
    const char *hex = "0123456789ABCDEF";
    char s[3];
    s[0] = hex[(v >> 4) & 0xF];
    s[1] = hex[v & 0xF];
    s[2] = '\0';
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

static void print_dec(uint32_t n)
{
    char buf[16];
    int i = 0;

    if (n == 0) {
        console_putc('0');
        return;
    }

    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i > 0) {
        console_putc(buf[--i]);
    }
}

static uint32_t cluster_to_lba(uint32_t cluster)
{
    return data_start_lba + (cluster - 2) * bs.sectors_per_cluster;
}

static int read_cluster(uint32_t cluster, uint8_t *buffer)
{
    uint32_t lba = cluster_to_lba(cluster);

    for (uint32_t i = 0; i < bs.sectors_per_cluster; i++) {
        if (storage_read_sector(lba + i, buffer + i * bs.bytes_per_sector) != 0)
            return -1;
    }

    return 0;
}

int fat32_init(void)
{
    uint8_t sector[512];

    if (storage_read_sector(0, sector) != 0) {
        console_write("FAT32: failed to read sector 0\n");
        return -1;
    }

    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        console_write("FAT32: invalid boot sector signature\n");
        return -1;
    }

    bs = *(FAT32_BootSector *)sector;

    if (bs.bytes_per_sector == 0 || bs.sectors_per_cluster == 0 || bs.fat_size_32 == 0) {
        console_write("FAT32: invalid BPB values\n");
        return -1;
    }

    fat_start_lba = bs.hidden_sectors + bs.reserved_sectors;
    data_start_lba = bs.hidden_sectors + bs.reserved_sectors + (bs.num_fats * bs.fat_size_32);
    root_cluster = bs.root_cluster;

    console_write("FAT32 Initialized\n");
    console_write("Hidden sectors: \n");
    print_dec(bs.hidden_sectors);
    console_putc('\n');
    console_write("Root cluster: \n");
    print_dec(root_cluster);
    console_putc('\n');
    return 0;
}

void fat32_print_info(void)
{
    console_write("FAT32 INFO\n");

    console_write("Bytes/Sector: ");
    print_dec(bs.bytes_per_sector);
    console_putc('\n');

    console_write("Sectors/Cluster: ");
    print_dec(bs.sectors_per_cluster);
    console_putc('\n');

    console_write("Reserved Sectors: ");
    print_dec(bs.reserved_sectors);
    console_putc('\n');

    console_write("Number of FATs: ");
    print_dec(bs.num_fats);
    console_putc('\n');

    console_write("FAT Size 32: ");
    print_dec(bs.fat_size_32);
    console_putc('\n');

    console_write("Root Cluster: ");
    print_dec(root_cluster);
    console_putc('\n');

    console_write("FAT Start LBA: ");
    print_dec(fat_start_lba);
    console_putc('\n');

    console_write("Data Start LBA: ");
    print_dec(data_start_lba);
    console_putc('\n');

    console_write("FS Type: ");
    for (int i = 0; i < 8; i++) {
        char c = (char)bs.fs_type[i];
        if (c) console_putc(c);
    }
    console_putc('\n');
}

void fat32_list_root(void)
{
    uint8_t cluster_buf[4096];

    if (read_cluster(root_cluster, cluster_buf) != 0) {
        console_write("FAT32: failed to read root cluster\n");
        return;
    }

    console_write("ROOT FILES:\n");

    FAT32_DirEntry *entry = (FAT32_DirEntry *)cluster_buf;
    int count = (bs.sectors_per_cluster * bs.bytes_per_sector) / sizeof(FAT32_DirEntry);

    for (int i = 0; i < count; i++) {
        if (entry[i].name[0] == 0x00)
            break;

        if (entry[i].name[0] == 0xE5)
            continue;

        if (entry[i].attr == 0x0F)
            continue; // long file name entry

        for (int j = 0; j < 11; j++) {
            char c = (char)entry[i].name[j];
            if (c != ' ')
                console_putc(c);
        }

        console_write("  size=");
        print_dec(entry[i].file_size);
        console_putc('\n');
    }
}

// Helper function

static int fat32_find_root_entry_by_name(const char *name, FAT32_DirEntry *out_entry, int *out_index)
{
    uint8_t cluster_buf[4096];
    uint8_t target[11];

    fat32_make_83_name(name, target);

    if (read_cluster(root_cluster, cluster_buf) != 0) {
        console_write("Read root failed\n");
        return -1;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)cluster_buf;
    int count = (bs.sectors_per_cluster * bs.bytes_per_sector) / sizeof(FAT32_DirEntry);

    console_write("Searching for: ");
    fat32_print_name11(target);
    console_putc('\n');

    for (int i = 0; i < count; i++)
    {
        if (entry[i].name[0] == 0x00)
            break;

        if (entry[i].name[0] == 0xE5)
            continue;

        if (entry[i].attr == 0x0F)
            continue;

        console_write("Entry: ");
        fat32_print_name11(entry[i].name);
        console_putc('\n');

        int match = 1;
        for (int j = 0; j < 11; j++) {
            if (target[j] != entry[i].name[j]) {
                match = 0;
                break;
            }
        }

        if (match) {
            if (out_entry) *out_entry = entry[i];
            if (out_index) *out_index = i;
            return 0;
        }
    }

    return -1;
}

// Read or Load file

int fat32_read_file_simple(const char *name, uint8_t *buffer, uint32_t *out_size)
{
    FAT32_DirEntry dir;

    if (fat32_find_root_entry_by_name(name, &dir, 0) != 0) {
        console_write("File not found\n");
        return -1;
    }

    uint32_t cluster =
        ((uint32_t)dir.first_cluster_high << 16) |
        dir.first_cluster_low;

    if (read_cluster(cluster, buffer) != 0) {
        console_write("Read file cluster failed\n");
        return -1;
    }

    *out_size = dir.file_size;
    console_write("File loaded\n");
    return 0;
}

// Write or Save file

int fat32_write_file_simple(const char *name, const uint8_t *buffer, uint32_t size)
{
    uint8_t root_buf[4096];
    FAT32_DirEntry dir;
    int entry_index = -1;

    if (fat32_find_root_entry_by_name(name, &dir, &entry_index) != 0) {
        console_write("Existing file not found\n");
        return -1;
    }

    uint32_t cluster =
        ((uint32_t)dir.first_cluster_high << 16) |
        dir.first_cluster_low;

    uint32_t lba = cluster_to_lba(cluster);

    uint8_t sec[512];
    for (int i = 0; i < 512; i++) sec[i] = 0;

    uint32_t copy = (size > 512) ? 512 : size;
    for (uint32_t i = 0; i < copy; i++)
        sec[i] = buffer[i];

    if (storage_write_sector(lba, sec) != 0) {
        console_write("Write failed\n");
        return -1;
    }

    // re-read root cluster so we can update file size
    if (read_cluster(root_cluster, root_buf) != 0) {
        console_write("Read root for update failed\n");
        return -1;
    }

    FAT32_DirEntry *entry = (FAT32_DirEntry *)root_buf;
    entry[entry_index].file_size = copy;

    uint32_t root_lba = cluster_to_lba(root_cluster);
    if (storage_write_sector(root_lba, root_buf) != 0) {
        console_write("Root update failed\n");
        return -1;
    }

    console_write("File updated\n");
    return 0;
}

// Helper function
static void fat32_make_83_name(const char *src, uint8_t out[11])
{
    for (int i = 0; i < 11; i++)
        out[i] = ' ';

    int i = 0;
    int j = 0;

    // filename part
    while (src[i] && src[i] != '.' && j < 8) {
        char c = src[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[j++] = (uint8_t)c;
        i++;
    }

    // skip dot
    if (src[i] == '.')
        i++;

    // extension part
    j = 8;
    while (src[i] && j < 11) {
        char c = src[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[j++] = (uint8_t)c;
        i++;
    }
}

// Debug print
static void fat32_print_name11(const uint8_t name[11])
{
    for (int i = 0; i < 11; i++) {
        char c = (char)name[i];
        if (c == ' ')
            console_putc('_');
        else
            console_putc(c);
    }
}