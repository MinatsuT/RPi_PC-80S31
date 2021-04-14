//
// Minatsu d88 Library by Minatsu
// 7-Apr-2021
//
#ifndef __MD88_H_
#define __MD88_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
// Debug support facilities
//================================================================================
#define Debug 1
#if defined(Debug)
#define DP(...) printf(__VA_ARGS__)
#else
#define DP(...)
#endif

// ======================================================================
// Utilities
// ======================================================================
#define ZEROFILL(var) memset(&(var), 0, sizeof(var))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// ======================================================================
// d88 2D disk image format
// ======================================================================
#define MAX_DRIVE 2
#define NUM_TRACK 80
#define NUM_SECTOR 16
#define SECTOR_SIZE 256
#define SET_2BYTE(p, n)           \
    do {                          \
        (p)[0] = n & 0xff;        \
        (p)[1] = (n >> 8) & 0xff; \
    } while (0)
#define SET_4BYTE(p, n)            \
    do {                           \
        SET_2BYTE(p, n);           \
        SET_2BYTE(p + 2, n >> 16); \
    } while (0)
#define GET_2BYTE(p) ((p)[0] | (p)[1] << 8)
#define GET_4BYTE(p) (GET_2BYTE(p) | GET_2BYTE(p + 2) << 16)

typedef struct {
    char disk[17];
    uint8_t reserve[9];
    uint8_t write_protect;
    uint8_t disk_type;
    uint8_t disk_size[4];
    uint8_t track_offset[164][4];
} disk_hdr_t;

typedef struct {
    uint8_t c;
    uint8_t h;
    uint8_t r;
    uint8_t n;
    uint8_t num_sec[2];
    uint8_t dense;
    uint8_t del_flag;
    uint8_t status;
    uint8_t reserve[5];
    uint8_t size[2];
    uint8_t data[SECTOR_SIZE];
} sector_t;

static FILE *md_fp[MAX_DRIVE];
static disk_hdr_t md_hdr[MAX_DRIVE];
sector_t md_buf[MAX_DRIVE][NUM_SECTOR];

// ======================================================================
// Disk image I/O
// ======================================================================
#define MD_WRITE 1
#define MD_READ 2
#define md_write(dr, tr, sec, nsec, buf) md_access(dr, tr, sec, nsec, buf, MD_WRITE)
#define md_read(dr, tr, sec, nsec, buf) md_access(dr, tr, sec, nsec, buf, MD_READ)

// ----------------------------------------------------------------------
// Read and write (Note: The sector number starts with 0, not 1.)
// ----------------------------------------------------------------------
int md_access(uint8_t drive, uint8_t tr, uint8_t sec, uint8_t num_sec, uint8_t *buf, uint8_t rw) {
    assert(rw == 1 || rw == 2);

    if (!(drive < MAX_DRIVE)) {
        DP("Illegal drive: %d\n", drive);
        return -1;
    }

    if (!(md_fp[drive] != NULL)) {
        DP("No disk: %d\n", drive);
        return -1;
    }

    if (!(tr < NUM_TRACK)) {
        DP("Illegal track: %d\n", tr);
        return -1;
    }

    // We don't check sector number.

    if (!num_sec) {
        // Do nothing
        return 0;
    }

    if (rw == MD_WRITE && md_hdr[drive].write_protect) {
        DP("Write protected.");
        return -1;
    }

    fseek(md_fp[drive], GET_4BYTE(md_hdr[drive].track_offset[tr]), SEEK_SET);
    fread(md_buf[drive], sizeof(md_buf[drive]), 1, md_fp[drive]);

    int c = tr / 2;
    int h = tr % 2;
    int n = 1;
    for (int i = 0; i < num_sec; i++) {
        int r = sec + 1 + i;
        // Search sector
        int j;
        for (j = 0; j < NUM_SECTOR; j++) {
            if (md_buf[drive][j].c == c && md_buf[drive][j].h == h && md_buf[drive][j].r == r && md_buf[drive][j].n == n) {
                break;
            }
        }
        if (j == NUM_SECTOR) {
            DP("Cannot find sector: Drive=%d C=%d H=%d R=%d N=%d.\n", drive, c, h, r, n);
            return -1; // No such sector
        }
        if (rw == MD_WRITE) {
            DP("Write sector: ");
            memcpy(md_buf[drive][j].data, &buf[SECTOR_SIZE * i], SECTOR_SIZE);
        } else {
            DP("Read sector: ");
            memcpy(&buf[SECTOR_SIZE * i], md_buf[drive][j].data, SECTOR_SIZE);
        }
        DP("Drive=%d C=%d H=%d R=%d N=%d.\r", drive, c, h, r, n);
    }
    DP("\n");
    if (rw == MD_WRITE) {
        if (fseek(md_fp[drive], GET_4BYTE(md_hdr[drive].track_offset[tr]), SEEK_SET)) {
            perror("Seek failed.");
            return -1;
        }
        int ret = fwrite(md_buf[drive], sizeof(md_buf[drive]), 1, md_fp[drive]);
        if (ret != 1) {
            perror("Write failed.");
            return -1;
        }
    }

    fflush(md_fp[drive]);
    return 0;
}

// ----------------------------------------------------------------------
// Format disk image
// ----------------------------------------------------------------------
int md_format(uint8_t drive) {
    if (!(drive < MAX_DRIVE)) {
        DP("Illegal drive: %d\n", drive);
        return -1;
    }

    if (!(md_fp[drive] != NULL)) {
        DP("No disk: %d\n", drive);
        return -1;
    }

    if (md_hdr[drive].write_protect) {
        DP("Write protected.");
        return -1;
    }

    int ret;

    ZEROFILL(md_hdr[drive]);
    SET_4BYTE(md_hdr[drive].disk_size, sizeof(disk_hdr_t) + sizeof(sector_t) *NUM_SECTOR* NUM_TRACK);
    for (int i = 0; i < NUM_TRACK; i++) {
        SET_4BYTE(md_hdr[drive].track_offset[i], sizeof(disk_hdr_t) + sizeof(sector_t) * NUM_SECTOR * i);
    }
    if (fseek(md_fp[drive], 0, SEEK_SET)) {
        perror("Format: seek failed.");
        return -1;
    }
    ret = fwrite((void *)&md_hdr[drive], sizeof(md_hdr[drive]), 1, md_fp[drive]);
    if (ret != 1) {
        perror("Format: write failed.");
        return -1;
    }

    ZEROFILL(md_buf[drive]);
    for (int i = 0; i < NUM_TRACK; i++) {
        for (int j = 0; j < NUM_SECTOR; j++) {
            md_buf[drive][j].c = i / 2;
            md_buf[drive][j].h = i % 2;
            md_buf[drive][j].r = j + 1;
            md_buf[drive][j].n = 1;
            SET_2BYTE(md_buf[drive][j].num_sec, NUM_SECTOR);
            md_buf[drive][j].dense = 1;
            SET_2BYTE(md_buf[drive][j].size, SECTOR_SIZE);
            DP("Drive=%d C=%d H=%d R=%d N=%d.\n", drive, md_buf[drive][j].c, md_buf[drive][j].h, md_buf[drive][j].r, md_buf[drive][j].n);
        }
        ret = fwrite(md_buf[drive], sizeof(md_buf[drive]), 1, md_fp[drive]);
        if (ret != 1) {
            perror("Write failed.");
            return -1;
        }
    }

    fflush(md_fp[drive]);
    return 0;
}

// ======================================================================
// Initialize and finalize
// ======================================================================
void md_close(uint8_t drive) {
    if (md_fp[drive] == NULL) {
        return;
    }
    fclose(md_fp[drive]);
    md_fp[drive] = NULL;
}

int md_open(uint8_t drive, char *fname) {
    assert(drive < MAX_DRIVE);
    assert(fname != NULL);

    DP("MD88: Open [%s]\n", fname);

    FILE *fp = md_fp[drive] = fopen(fname, "r+");
    if (fp == NULL) {
        fprintf(stderr, "MD88: Cannot open [%s]\n", fname);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        DP("New disk\n");
    } else {
        fseek(fp, 0, SEEK_SET);
        fread(&md_hdr[drive], sizeof(disk_hdr_t), 1, fp);

        DP("Disk=[%s]\n", md_hdr[drive].disk);
        DP("Disk Size=%d\n", md_hdr[drive].disk_size);
        DP("Track0 ofs=%x\n", md_hdr[drive].track_offset[0]);
    }

    return 0;
}

void MD_Quit() {
    for (int i = 0; i < MAX_DRIVE; i++) {
        if (md_fp[i] == NULL) {
            continue;
        }
        fclose(md_fp[i]);
    }
}

void MD_Init() {
    for (int i = 0; i < MAX_DRIVE; i++) {
        md_fp[i] = NULL;
    }
}

#ifdef __cplusplus
}
#endif

#endif // __MD88_H_
