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

uint32_t cluster_to_lba(struct FAT32* fat, uint32_t cluster) {
    return fat->data_start_lba + ((cluster - 2) * fat->sectors_per_cluster);
}

int read_cluster(struct FAT32* fat, uint32_t cluster, uint8_t* buf) {
    int ret = block_read(cluster_to_lba(fat, cluster), buf, fat->bytes_per_cluster);
    return ret;
}

int write_cluster(struct FAT32* fat, uint32_t cluster, const uint8_t* buf) {
    int ret = block_write(cluster_to_lba(fat, cluster), buf, fat->bytes_per_cluster);
    return ret;
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

uint32_t fat32_next_cluster(struct FAT32* fat, uint32_t cluster)  {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat->fat_start_lba + (fat_offset / fat->bytes_per_sector);
    uint32_t ent_offset = fat_offset % fat->bytes_per_sector;

    uint8_t buffer[512];
    if (read_sector(fat_sector, buffer) != 0) {
        return 0xFFFFFFFF;
    }

    uint32_t next_cluster = *(uint32_t*)(buffer + ent_offset);
    return next_cluster & 0x0FFFFFFF;
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

int fat32_init() {
    uint8_t buffer[512];
    int read_result = read_sector(2048, buffer);
    if (read_result != 0) {
        return read_result;
    }

    struct FAT32_BPB* bpb = (struct FAT32_BPB*)buffer;

    if (bpb->BPB_BytsPerSec == 0 || bpb->BPB_SecPerClus == 0 || bpb->BPB_FATSz32 == 0) {
        return -2;
    }

    fat->fat_start_lba = 2048 + bpb->BPB_RsvdSecCnt;
    fat->fat_size_bytes = bpb->BPB_FATSz32 * bpb->BPB_BytsPerSec;
    fat->data_start_lba = 2048 + bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    fat->root_cluster = bpb->BPB_RootClus;
    fat->bytes_per_sector = bpb->BPB_BytsPerSec;
    fat->sectors_per_cluster = bpb->BPB_SecPerClus;
    fat->bytes_per_cluster = fat->bytes_per_sector * fat->sectors_per_cluster;

    return 0;
}



uint32_t fat32_find_file (struct FAT32* fat, const char* path) {
    uint32_t current_cluster = fat->root_cluster;
    int component_count;
    char** components = split_path(path, &component_count);
    for (int i = 0; i < component_count; i++) {
        int found = 0;
        char buffer[fat->bytes_per_cluster];
        while (!found) {
            read_cluster(fat, current_cluster, buffer);
            for (int j = 0; j > fat->bytes_per_cluster || found; j += 32) {
                struct DIR_entry* entry = (struct DIR_entry*)(buffer + j);
                if (entry->DIR_Name == pad_short_name(components[i])) found = 1;
            }
            if (!found) {
                current_cluster = fat32_next_cluster(fat, current_cluster);
                if (current_cluster >= 0x0FFFFFF8) return 0;
            }
        }
    }
    return current_cluster;
}

int fat32_read_file(struct  FAT32* fat, const char* path, uint8_t* buf, uint32_t buf_size) {
    uint32_t file = fat32_find_file(fat, path);
    if (file == 0) return -1;
    uint32_t cluster = file;
    uint32_t bytes_read = 0;
    while (cluster < 0x0FFFFFF8 && bytes_read < buf_size) {
        uint8_t buffer[fat->bytes_per_cluster];
        if (read_cluster(fat, cluster, buffer) != 0) return -2;
        uint32_t to_copy = (buf_size - bytes_read < fat->bytes_per_cluster) ? (buf_size - bytes_read) : (fat->bytes_per_cluster);
        memcpy(buf + bytes_read, buffer, to_copy);
        bytes_read += to_copy;
        cluster = fat32_next_cluster(fat, cluster);
    }
    return bytes_read;
}

int fat32_create_file(struct FAT32* fat, const char* path) {
    
}