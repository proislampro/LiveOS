#include <stdint.h>

static struct FAT32 fat_instance;
struct FAT32* fat = &fat_instance;

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

uint32_t cluster_to_lba(struct FAT32* fat, uint32_t cluster) {
    return fat->data_start_lba + ((cluster - 2) * fat->sectors_per_cluster);
}

int read_cluster(struct FAT32* fat, uint32_t cluster, uint8_t* buf) {
    return block_read(cluster_to_lba(fat, cluster), buf, fat->sectors_per_cluster);
}

int write_cluster(struct FAT32* fat, uint32_t cluster, const uint8_t* buf) {
    return block_write(cluster_to_lba(fat, cluster), buf, fat->sectors_per_cluster);
}

static void pad_short_name(const char* name, char out[11]) {
    int i = 0;
    int j = 0;

    while (name[i] == '/') i++;

    while (name[i] && name[i] != '.' && j < 8) {
        out[j++] = (name[i] >= 'a' && name[i] <= 'z') ? (name[i] - 32) : name[i];
        i++;
    }

    while (j < 8) out[j++] = ' ';

    if (name[i] == '.') i++;

    for (int k = 0; k < 3; k++) {
        if (name[i + k]) {
            char c = name[i + k];
            out[j++] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        } else {
            out[j++] = ' ';
        }
    }
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

uint32_t fat32_find_file(struct FAT32* fat, const char* path) {
    char target[11];
    pad_short_name(path, target);

    uint32_t current_cluster = fat->root_cluster;

    while (current_cluster < 0x0FFFFFF8) {
        uint8_t buffer[4096];
        if (fat->bytes_per_cluster > sizeof(buffer)) return 0;
        if (read_cluster(fat, current_cluster, buffer) != 0) return 0;

        for (uint32_t j = 0; j + 32 <= fat->bytes_per_cluster; j += 32) {
            struct DIR_entry* entry = (struct DIR_entry*)(buffer + j);

            if ((uint8_t)entry->DIR_Name[0] == 0x00) {
                return 0;
            }
            if ((uint8_t)entry->DIR_Name[0] == 0xE5 || entry->DIR_Attr == 0x0F) {
                continue;
            }

            int same = 1;
            for (int n = 0; n < 11; n++) {
                if (entry->DIR_Name[n] != target[n]) {
                    same = 0;
                    break;
                }
            }

            if (same) {
                return ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
            }
        }

        current_cluster = fat32_next_cluster(fat, current_cluster);
    }

    return 0;
}

int fat32_read_file(struct FAT32* fat, const char* path, uint8_t* buf, uint32_t buf_size) {
    uint32_t file = fat32_find_file(fat, path);
    if (file == 0) return -1;

    uint32_t cluster = file;
    uint32_t bytes_read = 0;

    while (cluster < 0x0FFFFFF8 && bytes_read < buf_size) {
        uint8_t buffer[4096];
        if (fat->bytes_per_cluster > sizeof(buffer)) return -3;
        if (read_cluster(fat, cluster, buffer) != 0) return -2;

        uint32_t to_copy = fat->bytes_per_cluster;
        if (buf_size - bytes_read < to_copy) {
            to_copy = buf_size - bytes_read;
        }

        memcpy(buf + bytes_read, buffer, to_copy);
        bytes_read += to_copy;
        cluster = fat32_next_cluster(fat, cluster);
    }

    if (bytes_read < buf_size) {
        buf[bytes_read] = '\0';
    }

    return bytes_read;
}

int fat32_create_file(struct FAT32* fat, const char* path) {
    (void)fat;
    (void)path;
    return -1;
}
