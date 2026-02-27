#include <stdint.h>

struct FAT32* fat;

struct __attribute__((__packed__)) FAT32_BPB {
    uint8_t  BS_jmpBoot[1];
    uint8_t  BS_OEMName[3];
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

    // FAT32 Extended Fields
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
    uint8_t  BS_VolLab[14];
    uint8_t  BS_FilSysType[3];
};

struct FAT32 {
    uint32_t partition_start_lba;
    uint32_t fat_start_lba;
    uint32_t data_start_lba;
    uint16_t bytes_per_sector;
    uint8_t  sector_per_cluster;
    uint32_t root_cluster;
};

typedef struct __attribute__((packed))
{
    char     name[11];
    uint8_t  attr;
    uint8_t  ntres;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t first_cluster_high;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t first_cluster_low;
    uint32_t size;

} DirEntry;

typedef struct __attribute__((packed)) {
    uint8_t order;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster_low;
    uint16_t name3[2];
} LFNEntry;

uint32_t cluster_to_lba(struct FAT32* fat, uint32_t cluster) {
    return fat->data_start_lba + ((cluster - 2) * fat->sector_per_cluster);
}

int read_cluster(struct FAT32* fat, uint32_t cluster, uint8_t* buf) {
    uint32_t lba = cluster_to_lba(fat, cluster);
    for (uint8_t i = 0; i < fat->sector_per_cluster; i++) {
        int result = read_sector(lba + i, buf + (i * fat->bytes_per_sector));
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

int write_cluster(struct FAT32* fat, uint32_t cluster, const uint8_t* buf) {
    uint32_t lba = cluster_to_lba(fat, cluster);
    for (uint8_t i = 0; i < fat->sector_per_cluster; i++) {
        int result = write_sector(lba + i, buf + (i * fat->bytes_per_sector));
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

char* format_short_name(const char* raw_name) {
    static char formatted[13];
    int j = 0;
    for (int i = 0; i < 11; i++) {
        if (raw_name[i] != ' ') {
            if (i == 8) {
                formatted[j++] = '.';
            }
            formatted[j++] = raw_name[i];
        }
    }
    formatted[j] = '\0';
    return formatted;
}

char* pad_short_name(const char* name) {
    static char padded[11];
    int i = 0, j = 0;
    while (name[i] && name[i] != '.' && j < 8) {
        padded[j++] = name[i++];
    }
    while (j < 8) {
        padded[j++] = ' ';
    }
    if (name[i] == '.') {
        i++;
        for (int k = 0; k < 3; k++) {
            padded[j++] = name[i + k] ? name[i + k] : ' ';
        }
    } else {
        for (int k = 0; k < 3; k++) {
            padded[j++] = ' ';
        }
    }
    return padded;
}

char** split_path(const char* path, int* count) {
    char** components;
    int component_count = 0;
    const char* start = path;
    while (*start) {
        while (*start == '/') start++;
        if (*start == 0) break;

        const char* end = start;
        while (*end && *end != '/') end++;

        int length = end - start;
        strncpy(components[component_count], start, length);
        components[component_count][length] = '\0';
        component_count++;

        start = end;
    }
    *count = component_count;
    return components;
}

uint32_t fat32_read_fat_entry(struct FAT32* fat, uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat->fat_start_lba + (fat_offset / fat->bytes_per_sector);
    uint32_t ent_offset = fat_offset % fat->bytes_per_sector;

    uint8_t buffer[512];
    if (read_sector(fat_sector, buffer) != 0) {
        return 0x0FFFFFFF; // Return end of chain on error
    }
    return *(uint32_t*)(buffer + ent_offset) & 0x0FFFFFFF;
}

void lfn_extract(LFNEntry* lfn, char* out_name, int offset) {
    for (int i = 0; i < 5; i++) {
        out_name[offset + i] = (char)lfn->name1[i];
    }
    for (int i = 0; i < 6; i++) {
        out_name[offset + 5 + i] = (char)lfn->name2[i];
    }
    for (int i = 0; i < 2; i++) {
        out_name[offset + 11 + i] = (char)lfn->name3[i];
    }
    return;
}

int fat32_init() {
    uint8_t buffer[512];
    int read_result = read_sector(2048, buffer);
    if (read_result != 0) {
        return read_result;
    }

    struct FAT32_BPB* bpb = (struct FAT32_BPB*)buffer;

    if (strncmp((const char*)bpb->BS_FilSysType, "FAT32   ", 8) != 0) {
        return -2;
    }

    if (bpb->BPB_BytsPerSec == 0 || bpb->BPB_SecPerClus == 0 || bpb->BPB_FATSz32 == 0) {
        return -3;
    }

    fat->fat_start_lba = 2048 + bpb->BPB_RsvdSecCnt;
    fat->data_start_lba = 2048 + bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    fat->root_cluster = bpb->BPB_RootClus;
    fat->bytes_per_sector = bpb->BPB_BytsPerSec;
    fat->sector_per_cluster = bpb->BPB_SecPerClus;

    return 0;
}



uint32_t fat32_find_file (struct FAT32* fat, const char* path) {
    uint32_t current_cluster = fat->root_cluster;
    int component_count;
    char** components = split_path(path, &component_count);
    for (int i = 0; i < component_count; i++) {
        uint32_t cluster = current_cluster;
        int found = 0;

        while (cluster < 0x0FFFFFF8) {
            uint8_t buffer[fat->bytes_per_sector * fat->sector_per_cluster];

            if (read_cluster(fat, cluster, buffer) != 0) return 0;

            char lfn_name[256];
            memset(lfn_name, 0, sizeof(lfn_name));
            int lfn_length = 0;

            for (int offset = 0; offset < fat->bytes_per_sector * fat->sector_per_cluster; offset += sizeof(DirEntry)) {
                DirEntry* entry = (DirEntry*)(buffer + offset);
                if (entry->name[0] == 0x00) break;
                if (entry->name[0] == 0xE5) {
                    lfn_length = 0;
                    continue;
                }
                if (entry->attr == 0x0F) {
                    LFNEntry* lfn = (LFNEntry*)entry;
                    int order = (lfn->order & 0x1F) - 1;
                    lfn_extract(lfn, lfn_name, order * 13);
                    lfn_length = 1;
                    continue;
                }
                char name[256];
                if (lfn_length) strcpy(name, lfn_name);
                else strcpy(name, format_short_name(entry->name));
                if (strcmp(name, components[i]) == 0) {
                    current_cluster = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
                    found = 1;
                    lfn_length = 0;
                    break;
                }
                lfn_length = 0;
            }
            if (found) break;
            cluster = fat32_read_fat_entry(fat, cluster);
        }
        if (!found) return 0;
    }
    return current_cluster;
}

int fat32_read_file(struct  FAT32* fat, const char* path, uint8_t* buf, uint32_t buf_size) {
    uint32_t file = fat32_find_file(fat, path);
    if (file == 0) return -1;
    uint32_t cluster = file;
    uint32_t bytes_read = 0;
    while (cluster < 0x0FFFFFF8 && bytes_read < buf_size) {
        uint8_t buffer[fat->bytes_per_sector * fat->sector_per_cluster];
        if (read_cluster(fat, cluster, buffer) != 0) return -2;
        uint32_t to_copy = (buf_size - bytes_read < fat->bytes_per_sector * fat->sector_per_cluster) ? (buf_size - bytes_read) : (fat->bytes_per_sector * fat->sector_per_cluster);
        memcpy(buf + bytes_read, buffer, to_copy);
        bytes_read += to_copy;
        cluster = fat32_read_fat_entry(fat, cluster);
    }
    return bytes_read;
}

int fat32_create_file(struct FAT32* fat, const char* path) {
    
}