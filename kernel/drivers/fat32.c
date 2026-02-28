#include <stdint.h>

struct __attribute__((__packed__)) FAT32_BPB {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t  BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t  BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t  BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];
};

struct FAT32 {
    uint32_t partition_start_lba;
    uint32_t fat_start_lba;
    uint32_t fat_size_bytes;
    uint32_t data_start_lba;
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint32_t bytes_per_cluster;
    uint32_t root_cluster;
};

struct DIR_entry {
    char     DIR_Name[11];
    uint8_t  DIR_Attr;
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
};

static struct FAT32 fat_instance;
struct FAT32* fat = &fat_instance;

uint32_t cluster_to_lba(struct FAT32* f, uint32_t cluster) {
    return f->data_start_lba + ((cluster - 2) * f->sectors_per_cluster);
}

int read_cluster(struct FAT32* f, uint32_t cluster, uint8_t* buf) {
    return block_read(cluster_to_lba(f, cluster), buf, f->bytes_per_cluster);
}

static void pad_short_name(const char* name, char out[11]) {
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
    if (read_sector(fat_sector, buffer) != 0) return 0x0FFFFFFF;
    return (*(uint32_t*)(buffer + ent_offset)) & 0x0FFFFFFF;
}

int fat32_init() {
    uint8_t buffer[512];
    int read_result = read_sector(2048, buffer);
    if (read_result != 0) return read_result;

    struct FAT32_BPB* bpb = (struct FAT32_BPB*)buffer;

    if (bpb->BPB_BytsPerSec == 0 || bpb->BPB_SecPerClus == 0 || bpb->BPB_FATSz32 == 0) return -2;

    fat->partition_start_lba = 2048;
    fat->fat_start_lba = fat->partition_start_lba + bpb->BPB_RsvdSecCnt;
    fat->fat_size_bytes = bpb->BPB_FATSz32 * bpb->BPB_BytsPerSec;
    fat->data_start_lba = fat->partition_start_lba + bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    fat->root_cluster = bpb->BPB_RootClus;
    fat->bytes_per_sector = bpb->BPB_BytsPerSec;
    fat->sectors_per_cluster = bpb->BPB_SecPerClus;
    fat->bytes_per_cluster = fat->bytes_per_sector * fat->sectors_per_cluster;

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
            uint8_t cluster_data[4096]; 
            if (read_cluster(f, current_cluster, cluster_data) != 0) return -3;
            for (uint32_t off = 0; off < f->bytes_per_cluster; off += 32) {
                struct DIR_entry* entry = (struct DIR_entry*)(cluster_data + off);
                if (entry->DIR_Name[0] == 0x00) return -1;
                if ((uint8_t)entry->DIR_Name[0] == 0xE5 || (entry->DIR_Attr & 0x0F) == 0x0F) continue;
                if (memcmp(entry->DIR_Name, target_83, 11) == 0) {
                    current_cluster = ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
                    if (i < part_count - 1 && !(entry->DIR_Attr & 0x10)) return -2;
                    found = 1;
                    break;
                }
            }
            if (found) break;
            current_cluster = fat32_next_cluster(f, current_cluster);
        }
        if (!found) return 0;
    }
    return current_cluster;
}

int fat32_read_file(struct FAT32* f, const char* path, uint8_t* buf, uint32_t buf_size) {
    uint32_t cluster = fat32_find_file(f, path);
    if (cluster == 0) return -1;
    uint32_t total_read = 0;
    uint8_t cluster_buf[4096];
    while (cluster >= 2 && cluster < 0x0FFFFFF8 && total_read < buf_size) {
        if (read_cluster(f, cluster, cluster_buf) != 0) return -2;
        uint32_t to_copy = f->bytes_per_cluster;
        if (total_read + to_copy > buf_size) to_copy = buf_size - total_read;
        memcpy(buf + total_read, cluster_buf, to_copy);
        total_read += to_copy;
        cluster = fat32_next_cluster(f, cluster);
    }
    return total_read;
}