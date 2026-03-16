/*
 * MelonOS - ATA PIO Driver
 * Primary-master 28-bit LBA sector I/O
 */

#include "ata.h"
#include "io.h"

#define ATA_PRIMARY_IO        0x1F0
#define ATA_PRIMARY_CTRL      0x3F6

#define ATA_REG_DATA          0
#define ATA_REG_ERROR         1
#define ATA_REG_FEATURES      1
#define ATA_REG_SECCOUNT0     2
#define ATA_REG_LBA0          3
#define ATA_REG_LBA1          4
#define ATA_REG_LBA2          5
#define ATA_REG_HDDEVSEL      6
#define ATA_REG_COMMAND       7
#define ATA_REG_STATUS        7

#define ATA_CMD_READ_PIO      0x20
#define ATA_CMD_WRITE_PIO     0x30
#define ATA_CMD_CACHE_FLUSH   0xE7
#define ATA_CMD_IDENTIFY      0xEC

#define ATA_SR_BSY            0x80
#define ATA_SR_DRDY           0x40
#define ATA_SR_DF             0x20
#define ATA_SR_DRQ            0x08
#define ATA_SR_ERR            0x01

static int ata_present = 0;

static void ata_io_wait(void) {
    io_wait();
    io_wait();
    io_wait();
    io_wait();
}

static int ata_wait_not_busy(void) {
    uint8_t status;
    int timeout = 100000;

    while (timeout-- > 0) {
        status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        if ((status & ATA_SR_BSY) == 0) {
            return 0;
        }
    }

    return -1;
}

static int ata_wait_drq(void) {
    uint8_t status;
    int timeout = 100000;

    while (timeout-- > 0) {
        status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            return -1;
        }
        if (status & ATA_SR_DF) {
            return -1;
        }
        if ((status & ATA_SR_BSY) == 0 && (status & ATA_SR_DRQ) != 0) {
            return 0;
        }
    }

    return -1;
}

int ata_init(void) {
    uint8_t status;
    uint16_t identify[256];

    outb(ATA_PRIMARY_CTRL, 0x02);

    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
    ata_io_wait();

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
    if (status == 0) {
        ata_present = 0;
        return -1;
    }

    if (ata_wait_not_busy() != 0) {
        ata_present = 0;
        return -1;
    }

    if (inb(ATA_PRIMARY_IO + ATA_REG_LBA1) != 0 || inb(ATA_PRIMARY_IO + ATA_REG_LBA2) != 0) {
        ata_present = 0;
        return -1;
    }

    if (ata_wait_drq() != 0) {
        ata_present = 0;
        return -1;
    }

    for (int index = 0; index < 256; index++) {
        identify[index] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }

    (void)identify;
    ata_present = 1;
    return 0;
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    uint16_t *words = (uint16_t *)buffer;

    if (!ata_present || buffer == 0 || lba > 0x0FFFFFFF) {
        return -1;
    }

    if (ata_wait_not_busy() != 0) {
        return -1;
    }

    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + ATA_REG_FEATURES, 0x00);
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    if (ata_wait_drq() != 0) {
        return -1;
    }

    for (int index = 0; index < 256; index++) {
        words[index] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }

    ata_io_wait();
    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t *buffer) {
    const uint16_t *words = (const uint16_t *)buffer;

    if (!ata_present || buffer == 0 || lba > 0x0FFFFFFF) {
        return -1;
    }

    if (ata_wait_not_busy() != 0) {
        return -1;
    }

    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + ATA_REG_FEATURES, 0x00);
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    if (ata_wait_drq() != 0) {
        return -1;
    }

    for (int index = 0; index < 256; index++) {
        outw(ATA_PRIMARY_IO + ATA_REG_DATA, words[index]);
    }

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    if (ata_wait_not_busy() != 0) {
        return -1;
    }

    ata_io_wait();
    return 0;
}