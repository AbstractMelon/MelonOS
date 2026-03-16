/*
 * MelonOS - Persistent Filesystem
 * Flat filesystem stored on ATA disk sectors
 */

#include "fs.h"
#include "ata.h"
#include "string.h"

#define FS_MAGIC                0x4D465331u /* MFS1 */
#define FS_VERSION              1u

#define FS_START_LBA            2048u
#define FS_TOTAL_SECTORS        3072u

#define FS_SUPERBLOCK_SECTOR    0u
#define FS_INODE_BITMAP_SECTOR  1u
#define FS_DATA_BITMAP_SECTOR   2u
#define FS_INODE_TABLE_START    3u
#define FS_INODE_TABLE_SECTORS  16u
#define FS_DATA_START_SECTOR    (FS_INODE_TABLE_START + FS_INODE_TABLE_SECTORS)

#define FS_MAX_INODES           96u
#define FS_MAX_DIRECT_BLOCKS    7u

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t fs_start_lba;
    uint32_t fs_total_sectors;
    uint32_t inode_bitmap_sector;
    uint32_t data_bitmap_sector;
    uint32_t inode_table_start;
    uint32_t inode_table_sectors;
    uint32_t data_start_sector;
    uint32_t max_inodes;
    uint32_t max_data_blocks;
    uint32_t reserved[5];
} fs_superblock_t;

typedef struct __attribute__((packed)) {
    uint8_t used;
    char name[FS_NAME_MAX_LEN + 1];
    uint32_t size;
    uint32_t direct[FS_MAX_DIRECT_BLOCKS];
} fs_inode_t;

static int fs_ready = 0;
static fs_superblock_t superblock;

static uint8_t inode_bitmap[ATA_SECTOR_SIZE];
static uint8_t data_bitmap[ATA_SECTOR_SIZE];
static uint8_t inode_table_raw[FS_INODE_TABLE_SECTORS * ATA_SECTOR_SIZE];

static uint32_t fs_max_data_blocks(void) {
    return FS_TOTAL_SECTORS - FS_DATA_START_SECTOR;
}

static uint32_t fs_sector_lba(uint32_t relative_sector) {
    return FS_START_LBA + relative_sector;
}

static fs_inode_t *fs_inode_table(void) {
    return (fs_inode_t *)inode_table_raw;
}

static int fs_is_valid_name(const char *name) {
    size_t len;

    if (name == 0 || name[0] == '\0') {
        return 0;
    }

    len = strlen(name);
    if (len == 0 || len > FS_NAME_MAX_LEN) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (name[i] == ' ' || name[i] == '/' || name[i] == '\\') {
            return 0;
        }
    }

    return 1;
}

static int bitmap_get(const uint8_t *bitmap, uint32_t index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static void bitmap_set(uint8_t *bitmap, uint32_t index, int value) {
    if (value) {
        bitmap[index / 8] |= (uint8_t)(1u << (index % 8));
    } else {
        bitmap[index / 8] &= (uint8_t)~(1u << (index % 8));
    }
}

static int fs_read_metadata(void) {
    uint8_t superblock_sector[ATA_SECTOR_SIZE];

    if (ata_read_sector(fs_sector_lba(FS_SUPERBLOCK_SECTOR), superblock_sector) != 0) {
        return -1;
    }

    memcpy(&superblock, superblock_sector, sizeof(superblock));

    if (ata_read_sector(fs_sector_lba(FS_INODE_BITMAP_SECTOR), inode_bitmap) != 0) {
        return -1;
    }

    if (ata_read_sector(fs_sector_lba(FS_DATA_BITMAP_SECTOR), data_bitmap) != 0) {
        return -1;
    }

    for (uint32_t sector = 0; sector < FS_INODE_TABLE_SECTORS; sector++) {
        if (ata_read_sector(fs_sector_lba(FS_INODE_TABLE_START + sector),
                            &inode_table_raw[sector * ATA_SECTOR_SIZE]) != 0) {
            return -1;
        }
    }

    return 0;
}

static int fs_write_metadata(void) {
    uint8_t superblock_sector[ATA_SECTOR_SIZE];

    memset(superblock_sector, 0, sizeof(superblock_sector));
    memcpy(superblock_sector, &superblock, sizeof(superblock));

    if (ata_write_sector(fs_sector_lba(FS_SUPERBLOCK_SECTOR), superblock_sector) != 0) {
        return -1;
    }

    if (ata_write_sector(fs_sector_lba(FS_INODE_BITMAP_SECTOR), inode_bitmap) != 0) {
        return -1;
    }

    if (ata_write_sector(fs_sector_lba(FS_DATA_BITMAP_SECTOR), data_bitmap) != 0) {
        return -1;
    }

    for (uint32_t sector = 0; sector < FS_INODE_TABLE_SECTORS; sector++) {
        if (ata_write_sector(fs_sector_lba(FS_INODE_TABLE_START + sector),
                             &inode_table_raw[sector * ATA_SECTOR_SIZE]) != 0) {
            return -1;
        }
    }

    return 0;
}

static int fs_find_inode(const char *name) {
    fs_inode_t *inodes = fs_inode_table();

    for (uint32_t index = 0; index < FS_MAX_INODES; index++) {
        if (inodes[index].used && strcmp(inodes[index].name, name) == 0) {
            return (int)index;
        }
    }

    return -1;
}

static int fs_find_free_inode(void) {
    for (uint32_t index = 0; index < FS_MAX_INODES; index++) {
        if (!bitmap_get(inode_bitmap, index)) {
            return (int)index;
        }
    }
    return -1;
}

static int fs_find_free_data_block(void) {
    uint32_t max_blocks = superblock.max_data_blocks;

    for (uint32_t index = 0; index < max_blocks; index++) {
        if (!bitmap_get(data_bitmap, index)) {
            return (int)index;
        }
    }
    return -1;
}

static void fs_release_inode_blocks(fs_inode_t *inode) {
    if (inode == 0) {
        return;
    }

    for (uint32_t i = 0; i < FS_MAX_DIRECT_BLOCKS; i++) {
        uint32_t block = inode->direct[i];
        if (block < superblock.max_data_blocks) {
            bitmap_set(data_bitmap, block, 0);
        }
        inode->direct[i] = 0;
    }

    inode->size = 0;
}

int fs_init(void) {
    uint8_t superblock_sector[ATA_SECTOR_SIZE];

    if (ata_init() != 0) {
        fs_ready = 0;
        return -1;
    }

    if (ata_read_sector(fs_sector_lba(FS_SUPERBLOCK_SECTOR), superblock_sector) != 0) {
        fs_ready = 0;
        return -1;
    }

    memcpy(&superblock, superblock_sector, sizeof(superblock));

    if (superblock.magic != FS_MAGIC ||
        superblock.version != FS_VERSION ||
        superblock.fs_start_lba != FS_START_LBA ||
        superblock.fs_total_sectors != FS_TOTAL_SECTORS ||
        superblock.max_inodes != FS_MAX_INODES ||
        superblock.max_data_blocks != fs_max_data_blocks()) {
        fs_ready = 0;
        return -1;
    }

    if (fs_read_metadata() != 0) {
        fs_ready = 0;
        return -1;
    }

    fs_ready = 1;
    return 0;
}

int fs_is_ready(void) {
    return fs_ready;
}

int fs_format(void) {
    uint8_t zero_sector[ATA_SECTOR_SIZE];

    if (ata_init() != 0) {
        fs_ready = 0;
        return -1;
    }

    memset(zero_sector, 0, sizeof(zero_sector));
    memset(&superblock, 0, sizeof(superblock));
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    memset(data_bitmap, 0, sizeof(data_bitmap));
    memset(inode_table_raw, 0, sizeof(inode_table_raw));

    superblock.magic = FS_MAGIC;
    superblock.version = FS_VERSION;
    superblock.fs_start_lba = FS_START_LBA;
    superblock.fs_total_sectors = FS_TOTAL_SECTORS;
    superblock.inode_bitmap_sector = FS_INODE_BITMAP_SECTOR;
    superblock.data_bitmap_sector = FS_DATA_BITMAP_SECTOR;
    superblock.inode_table_start = FS_INODE_TABLE_START;
    superblock.inode_table_sectors = FS_INODE_TABLE_SECTORS;
    superblock.data_start_sector = FS_DATA_START_SECTOR;
    superblock.max_inodes = FS_MAX_INODES;
    superblock.max_data_blocks = fs_max_data_blocks();

    if (fs_write_metadata() != 0) {
        fs_ready = 0;
        return -1;
    }

    for (uint32_t sector = FS_DATA_START_SECTOR; sector < FS_TOTAL_SECTORS; sector++) {
        if (ata_write_sector(fs_sector_lba(sector), zero_sector) != 0) {
            fs_ready = 0;
            return -1;
        }
    }

    fs_ready = 1;
    return 0;
}

int fs_list(fs_file_info_t *files, size_t max_files, size_t *out_count) {
    fs_inode_t *inodes;
    size_t count = 0;

    if (!fs_ready || files == 0 || out_count == 0) {
        return -1;
    }

    if (fs_read_metadata() != 0) {
        return -1;
    }

    inodes = fs_inode_table();
    for (uint32_t index = 0; index < FS_MAX_INODES; index++) {
        if (!inodes[index].used) {
            continue;
        }

        if (count < max_files) {
            strcpy(files[count].name, inodes[index].name);
            files[count].size = inodes[index].size;
        }
        count++;
    }

    *out_count = count;
    return 0;
}

int fs_write_file(const char *name, const uint8_t *data, uint32_t size) {
    fs_inode_t *inodes;
    fs_inode_t *inode;
    int inode_index;
    uint32_t blocks_needed;
    uint8_t sector[ATA_SECTOR_SIZE];

    if (!fs_ready || (size > 0 && data == 0) || !fs_is_valid_name(name)) {
        return -1;
    }

    blocks_needed = (size == 0) ? 0 : ((size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE);
    if (blocks_needed > FS_MAX_DIRECT_BLOCKS) {
        return -1;
    }

    if (fs_read_metadata() != 0) {
        return -1;
    }

    inodes = fs_inode_table();
    inode_index = fs_find_inode(name);
    if (inode_index < 0) {
        inode_index = fs_find_free_inode();
        if (inode_index < 0) {
            return -1;
        }
    }

    inode = &inodes[inode_index];
    if (inode->used) {
        fs_release_inode_blocks(inode);
    }

    for (uint32_t block_index = 0; block_index < blocks_needed; block_index++) {
        int free_block = fs_find_free_data_block();
        uint32_t offset;
        uint32_t chunk;

        if (free_block < 0) {
            fs_release_inode_blocks(inode);
            return -1;
        }

        inode->direct[block_index] = (uint32_t)free_block;
        bitmap_set(data_bitmap, (uint32_t)free_block, 1);

        memset(sector, 0, sizeof(sector));
        offset = block_index * ATA_SECTOR_SIZE;
        chunk = size - offset;
        if (chunk > ATA_SECTOR_SIZE) {
            chunk = ATA_SECTOR_SIZE;
        }
        memcpy(sector, data + offset, chunk);

        if (ata_write_sector(fs_sector_lba(FS_DATA_START_SECTOR + (uint32_t)free_block), sector) != 0) {
            fs_release_inode_blocks(inode);
            return -1;
        }
    }

    inode->used = 1;
    memset(inode->name, 0, sizeof(inode->name));
    strncpy(inode->name, name, FS_NAME_MAX_LEN);
    inode->name[FS_NAME_MAX_LEN] = '\0';
    inode->size = size;
    bitmap_set(inode_bitmap, (uint32_t)inode_index, 1);

    if (fs_write_metadata() != 0) {
        return -1;
    }

    return 0;
}

int fs_read_file(const char *name, uint8_t *buffer, uint32_t buffer_size, uint32_t *out_size) {
    fs_inode_t *inodes;
    fs_inode_t *inode;
    int inode_index;
    uint8_t sector[ATA_SECTOR_SIZE];

    if (!fs_ready || buffer == 0 || out_size == 0 || !fs_is_valid_name(name)) {
        return -1;
    }

    if (fs_read_metadata() != 0) {
        return -1;
    }

    inodes = fs_inode_table();
    inode_index = fs_find_inode(name);
    if (inode_index < 0) {
        return -1;
    }

    inode = &inodes[inode_index];
    if (inode->size > buffer_size) {
        return -1;
    }

    for (uint32_t block_index = 0; block_index < FS_MAX_DIRECT_BLOCKS; block_index++) {
        uint32_t offset = block_index * ATA_SECTOR_SIZE;
        uint32_t chunk;

        if (offset >= inode->size) {
            break;
        }

        chunk = inode->size - offset;
        if (chunk > ATA_SECTOR_SIZE) {
            chunk = ATA_SECTOR_SIZE;
        }

        if (ata_read_sector(fs_sector_lba(FS_DATA_START_SECTOR + inode->direct[block_index]), sector) != 0) {
            return -1;
        }

        memcpy(buffer + offset, sector, chunk);
    }

    *out_size = inode->size;
    return 0;
}

int fs_delete_file(const char *name) {
    fs_inode_t *inodes;
    fs_inode_t *inode;
    int inode_index;

    if (!fs_ready || !fs_is_valid_name(name)) {
        return -1;
    }

    if (fs_read_metadata() != 0) {
        return -1;
    }

    inodes = fs_inode_table();
    inode_index = fs_find_inode(name);
    if (inode_index < 0) {
        return -1;
    }

    inode = &inodes[inode_index];
    fs_release_inode_blocks(inode);
    memset(inode, 0, sizeof(*inode));
    bitmap_set(inode_bitmap, (uint32_t)inode_index, 0);

    if (fs_write_metadata() != 0) {
        return -1;
    }

    return 0;
}

int fs_get_info(fs_info_t *info) {
    uint32_t used_inodes = 0;
    uint32_t free_data_blocks = 0;

    if (!fs_ready || info == 0) {
        return -1;
    }

    if (fs_read_metadata() != 0) {
        return -1;
    }

    for (uint32_t i = 0; i < superblock.max_inodes; i++) {
        if (bitmap_get(inode_bitmap, i)) {
            used_inodes++;
        }
    }

    for (uint32_t i = 0; i < superblock.max_data_blocks; i++) {
        if (!bitmap_get(data_bitmap, i)) {
            free_data_blocks++;
        }
    }

    info->total_sectors = superblock.fs_total_sectors;
    info->free_data_blocks = free_data_blocks;
    info->total_data_blocks = superblock.max_data_blocks;
    info->used_inodes = used_inodes;
    info->total_inodes = superblock.max_inodes;

    return 0;
}