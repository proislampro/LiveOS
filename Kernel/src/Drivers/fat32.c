#include <stdint.h>
#include <string.h>
#include <screen.h>
#include <ata.h>
#include <fat32.h>
#include <memory.h>

static struct FAT32 fat_instance;
struct FAT32* fat = &fat_instance;

uint32_t cluster_to_lba(struct FAT32* f, uint32_t cluster) {
    return f->data_start_lba + ((cluster - 2) * f->sectors_per_cluster);
}

int read_cluster(struct FAT32* f, uint32_t cluster, uint8_t* buf) {
    if (ata_read_sectors(0, 0, cluster_to_lba(f, cluster), f->sectors_per_cluster, buf) != 0) return -1;
    return 0;
}

void pad_short_name(const char* name, char out[11]) {
    memset(out, ' ', 11);
    int i = 0, j = 0;
    while (name[i] == '/') i++;
    while (name[i] && name[i] != '.' && j < 8) {
        char c = name[i++];
        out[j++] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    }
    const char* dot = strchr(name, '.');
    if (dot) {
        dot++;
        for (int k = 0; k < 3 && dot[k]; k++) {
            char c = dot[k];
            out[8 + k] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        }
    }
}

uint32_t fat32_next_cluster(struct FAT32* f, uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = f->fat_start_lba + (fat_offset / f->bytes_per_sector);
    uint32_t ent_offset = fat_offset % f->bytes_per_sector;
    uint8_t buffer[512];
    if (ata_read_sectors(0, 0, fat_sector, 1, buffer) != 0) return 0x0FFFFFFF;

    uint32_t next = (*(uint32_t*)(buffer + ent_offset)) & 0x0FFFFFFF;
    if (next >= 0x0FFFFFF8) return 0;  // End of cluster chain
    return next;
}

int fat32_init(struct FAT32* f, uint32_t partition_start_lba) {
    uint8_t buffer[512];
    int read_result = ata_read_sectors(0, 0, partition_start_lba, 1, buffer);
    if (read_result != 0) return read_result;

    struct FAT32_BPB* bpb = (struct FAT32_BPB*)buffer;

    if (bpb->BPB_BytsPerSec == 0 || bpb->BPB_SecPerClus == 0 || bpb->BPB_FATSz32 == 0) return -2;

    f->partition_start_lba = partition_start_lba;
    f->fat_start_lba = f->partition_start_lba + bpb->BPB_RsvdSecCnt;
    f->fat_size_bytes = bpb->BPB_FATSz32 * bpb->BPB_BytsPerSec;
    f->data_start_lba = f->partition_start_lba + bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    f->root_cluster = bpb->BPB_RootClus;
    f->bytes_per_sector = bpb->BPB_BytsPerSec;
    f->sectors_per_cluster = bpb->BPB_SecPerClus;
    f->bytes_per_cluster = f->bytes_per_sector * f->sectors_per_cluster;

    return 0;
}

int split_path(const char* path, char* buffer, char** parts) {
    int count = 0;
    char* ptr = buffer;
    int active = 0;
    for (int i = 0; path[i]; i++) {
        if (path[i] == '/') {
            if (active) {
                *ptr++ = '\0';
                active = 0;
            }
        } else {
            if (!active) {
                parts[count++] = ptr;
                active = 1;
            }
            *ptr++ = path[i];
        }
    }
    if (active) *ptr = '\0';
    return count;
}

uint32_t fat32_find_file(struct FAT32* f, const char* path) {
    char name_buf[256];
    char* parts[16];
    int part_count = split_path(path, name_buf, parts);
    if (part_count == 0) return f->root_cluster;

    uint32_t current_cluster = f->root_cluster;

    for (int i = 0; i < part_count; i++) {
        char target_83[11];
        pad_short_name(parts[i], target_83);
        int found = 0;

        while (current_cluster >= 2 && current_cluster < 0x0FFFFFF8) {
            uint32_t cluster_size = f->bytes_per_cluster;
            static uint8_t cluster_data[4096];
            if (cluster_size > sizeof(cluster_data)) return -4;
            if (read_cluster(f, current_cluster, cluster_data) != 0) return -3;

            for (uint32_t off = 0; off < cluster_size; off += 32) {
                struct DIR_entry* entry = (struct DIR_entry*)(cluster_data + off);

                if (entry->DIR_Name[0] == 0x00) break;  // end of directory entries

                if ((uint8_t)entry->DIR_Name[0] == 0xE5 || (entry->DIR_Attr & 0x0F) == 0x0F)
                    continue; // deleted or LFN

                if (memcmp(entry->DIR_Name, target_83, 11) == 0) {
                    current_cluster = ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
                    if (i < part_count - 1 && !(entry->DIR_Attr & 0x10)) return -2; // not a directory
                    found = 1;
                    break;
                }
            }

            if (found) break;

            uint32_t next = fat32_next_cluster(f, current_cluster);
            if (next == 0 || next >= 0x0FFFFFF8) break;
            current_cluster = next;
        }

        if (!found) return 0; // not found
    }

    return current_cluster;
}

int fat32_read_file(struct FAT32* f, const char* path, uint8_t* buf, uint32_t buf_size) {
    int32_t cluster = fat32_find_file(f, path);
    if (cluster <= 0) return cluster; // propagate error codes

    uint32_t total_read = 0;
    static uint8_t cluster_buf[4096]; // safe on .bss

    while (cluster >= 2 && cluster < 0x0FFFFFF8 && total_read < buf_size) {
        if (read_cluster(f, cluster, cluster_buf) != 0) return -3;

        uint32_t to_copy = f->bytes_per_cluster;
        if (total_read + to_copy > buf_size)
            to_copy = buf_size - total_read;

        memcpy(buf + total_read, cluster_buf, to_copy);
        total_read += to_copy;

        uint32_t next = fat32_next_cluster(f, cluster);
        if (next == 0 || next >= 0x0FFFFFF8) break;
        cluster = next;
    }

    return total_read;
}
