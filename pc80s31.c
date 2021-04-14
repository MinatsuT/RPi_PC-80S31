//
// PC-80S31 emulator by Minatsu
// 7-Apr-2021
//
#include "MGPIO.h"
#include "MD88.h"

//       Raspberry Pi's GPIO      PC
#define RD_DAT 4  // [11: 4] <--  PB7-PB0 Read DAT
#define WR_DAT 12 // [19:12] -->  PA0-PA7 Write DAT
#define RD_DAV 20 // [20]    <--  PC4 Read DAV (Data Valid: data has sent)
#define RD_RFD 21 // [21]    <--  PC5 Read RFD (Ready for Data: ready to receive data)
#define RD_DAC 22 // [22]    <--  PC6 Read DAC (Data Accepted: data has received)
#define RD_ATN 23 // [23]    <--  PC7 Read ATN (Attention: request for send data)
#define WR_DAV 24 // [24]    -->  PC0 Write DAV (Data Valid: data has sent)
#define WR_RFD 25 // [25]    -->  PC1 Write RFD (Ready for Data: ready to receive data)
#define WR_DAC 26 // [26]    -->  PC2 Write DAC (Data Accepted: data has received)
#define RD_RST 27 // [27]    <--  NRST Read RESET

// ======================================================================
// Utility
// ======================================================================
// Print binary
void printb(unsigned int v) {
    for (int i = 31; i >= 0; i--) {
        putchar(BIT(v, i) ? '1' : '0');
        if (!(i % 4) && i) {
            putchar('_');
        }
    }
}

// ======================================================================
// GPIO
// ======================================================================
static uint8_t rd_pins[] = {RD_DAT, 8, RD_DAV, 1, RD_RFD, 1, RD_DAC, 1, RD_ATN, 1, RD_RST, 1};
static uint8_t wr_pins[] = {WR_DAT, 8, WR_DAV, 1, WR_RFD, 1, WR_DAC, 1};
// Set GPIO input/output mode
void init_gpio() {
    for (int i = 0; i < sizeof(rd_pins); i += 2) {
        for (int j = rd_pins[i]; j < rd_pins[i] + rd_pins[i + 1]; j++) {
            func_sel(j, FUNC_INPUT);
            // usleep(10 * 1000);
            set_pud(j, PULL_DOWN);
            // usleep(10 * 1000);
        }
    }

    for (int i = 0; i < sizeof(wr_pins); i += 2) {
        for (int j = wr_pins[i]; j < wr_pins[i] + wr_pins[i + 1]; j++) {
            func_sel(j, FUNC_OUTPUT);
            // usleep(10 * 1000);
            gpio_write(j, 0, 1);
            // usleep(10 * 1000);
        }
    }
}

// Read DAT via GPIO
static inline uint8_t read_dat_gpio() {
    uint32_t d = gpio_read(0, 32);
    return BITS(d, RD_DAT, 8);
}

// Write DAT via GPIO
static inline void write_dat_gpio(uint8_t dat) {
    gpio_write(WR_DAT, dat & 0xff, 8);
}

// Wait until the signal becomes high
static inline void wait_high(uint8_t bit) {
    uint32_t d;
    do {
        d = gpio_read(bit, 1);
        assert(d < 2);
    } while (d == 0);
}

// Wait until the signal becomes low
static inline void wait_low(uint8_t bit) {
    uint32_t d;
    do {
        d = gpio_read(bit, 1);
        assert(d < 2);
    } while (d == 1);
}

// Print signal status
void sig_stat(char *mes) {
#ifdef DEBUG_PROTOCOL
    DP("%20s : ", mes);
    uint32_t d = gpio_read(0, 32);
    DP("ATN=%d DAV=%d RFD=%d DAC=%d [", BIT(d, RD_ATN), BIT(d, RD_DAV), BIT(d, RD_RFD), BIT(d, RD_DAC));
    printb(d >> 2);
    DP("]\n");
#endif
}

// ======================================================================
// Communication protocol
// ======================================================================
// #define DEBUG_PROTOCOL
#ifdef DEBUG_PROTOCOL
#define DP_PROTOCOL(...) DP(__VA_ARGS__)
#else
#define DP_PROTOCOL(...)
#endif

#define ASSERT_BIT(bit) gpio_write(bit, 1, 1);
#define DE_ASSERT_BIT(bit) gpio_write(bit, 0, 1);

// Send DAT
void send_dat(int num_dat, uint16_t dat) {
    DP_PROTOCOL("----------------------------------------------------------------------------------------\n");
    assert(num_dat >= 1 && num_dat <= 2);

    // Wait for RFD
    sig_stat("Wait for RFD");
    wait_high(RD_RFD);

    // Write DAT
    DP_PROTOCOL("Write DAT1: ");
    write_dat_gpio(dat);

    // Assert DAV
    DP_PROTOCOL("Assert DAV\n");
    ASSERT_BIT(WR_DAV)

    // Wait for DAC
    sig_stat("Wait for DAC");
    wait_high(RD_DAC);

    // Write second DAT
    if (num_dat == 2) {
        DP_PROTOCOL("Write DAT2: ");
        write_dat_gpio(dat >> 8);
    }

    // De-assert DAV
    DP_PROTOCOL("De-Assert DAV\n");
    DE_ASSERT_BIT(WR_DAV)

    // Wait de-assertion of DAC
    sig_stat("Wait DAC low");
    wait_low(RD_DAC);
    sig_stat("DAC low");
    DP_PROTOCOL("========================================================================================\n");
}

// Receive DAT
uint16_t receive_dat(int num_dat) {
    DP_PROTOCOL("----------------------------------------------------------------------------------------\n");
    assert(num_dat >= 1 && num_dat <= 2);

    // Assert RFD
    DP_PROTOCOL("Assert RFD\n");
    ASSERT_BIT(WR_RFD)

    // Wait for DAV
    sig_stat("Wait for DAV");
    wait_high(RD_DAV);
    sig_stat("Catch DAV");

    // De-assert RFD
    DP_PROTOCOL("De-Assert RFD\n");
    DE_ASSERT_BIT(WR_RFD)

    // Read DAT
    DP_PROTOCOL("Read DAT1: ");
    uint16_t dat = read_dat_gpio();

    // Assert DAC
    DP_PROTOCOL("Assert DAC\n");
    ASSERT_BIT(WR_DAC)

    // Wait de-assertion of DAV
    sig_stat("Wait DAV low");
    wait_low(RD_DAV);
    sig_stat("DAV low");

    // Read second DAT
    if (num_dat == 2) {
        DP_PROTOCOL("Read DAT2: ");
        dat |= read_dat_gpio() << 8;
    }

    // De-assert DAC
    DP_PROTOCOL("De-Assert DAC\n");
    DE_ASSERT_BIT(WR_DAC)

    DP_PROTOCOL("========================================================================================\n");
    return dat;
}

// Receive CMD
uint8_t read_cmd() {
    // Wait for ATN
    sig_stat("\nWait for ATN");
    wait_high(RD_ATN);
    sig_stat("Catch ATN");
    uint8_t ret = receive_dat(1);
    return ret;
}

// Receive sector data into the buffer
void receive_sector_data(int num_dat, int num_sec, uint8_t *buf) {
    assert(num_dat >= 1 && num_dat <= 2);

    for (int i = 0; i < SECTOR_SIZE * num_sec; i += num_dat) {
        uint16_t d = receive_dat(num_dat);
        buf[i] = d & 0xff;
        if (num_dat == 2) {
            buf[i + 1] = (d >> 8) & 0xff;
        }
    }
}

// Send sector data in the buffer
void send_sector_data(int num_dat, int num_sec, uint8_t *buf) {
    assert(num_dat >= 1 && num_dat <= 2);

    for (int i = 0; i < SECTOR_SIZE * num_sec; i += num_dat) {
        send_dat(num_dat, buf[i] | ((num_dat == 2) ? buf[i + 1] << 8 : 0));
    }
}

// ======================================================================
// Main
// ======================================================================
union {
    struct _result_stat {
        uint8_t is_error : 1;
        uint8_t _dummy : 5;
        uint8_t is_unread_buf : 1;
        uint8_t is_io_complete : 1;
    } bit;
    uint8_t dat;
} result_stat = {/*err*/ 0, 0, /*unread*/ 0, /*complete*/ 1};

uint16_t drive_stat = 0b00110011;
uint8_t num_sec, drive, tr, sec;
uint8_t buf[SECTOR_SIZE * NUM_SECTOR];

int main(int argc, char *argv[]) {
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    MGPIO_Init();
    init_gpio();
    MD_Init();

    int ret;
    if (argc > 1) {
        for (int i = 0; i < MAX(1, argc - 1); i++) {
            printf("Mount [%s] on Drive %d\n", argv[1 + i], i + 1);
            ret = md_open(i, argv[1 + i]);
            assert(ret == 0);
        }
    } else {
        printf("Usage: %0 disk1.d88 [disk2.d88]\n");
        exit(0);
    }

    sig_stat("Wait for RST");
    do {
#if 0
        // Wait for RST#
        DP("Waiting RESET\n");
        wait_low(RD_RST);
#endif
        // Wait for RST#
        wait_high(RD_RST);
        DP("Out of RESET\n");
    } while (0);
    sig_stat("Catch RST");

    while (1) {
        uint8_t cmd = read_cmd();
        DP("CMD: (%02x) ", cmd);
        switch (cmd) {
        case 0x00:
            DP("Initialize\n");
            result_stat.dat = 0x00;
            break;
        case 0x01:
            num_sec = receive_dat(1);
            drive = receive_dat(1);
            tr = receive_dat(1);
            sec = receive_dat(1) - 1; // Translate sector number.
            DP("Write Disk: num_sec=%d drive=%d tr=%d sec=%d\n", num_sec, drive, tr, sec + 1);
            receive_sector_data(1, num_sec, buf);
            if (md_write(drive, tr, sec, num_sec, buf) == 0) {
                result_stat.bit.is_error = 0;
            } else {
                result_stat.bit.is_error = 1;
            }
            break;
        case 0x02:
            num_sec = receive_dat(1);
            drive = receive_dat(1);
            tr = receive_dat(1);
            sec = receive_dat(1) - 1; // Translate sector number.
            DP("Read Disk: num_sec=%d drive=%d tr=%d sec=%d\n", num_sec, drive, tr, sec + 1);
            if (md_read(drive, tr, sec, num_sec, buf) == 0) {
                result_stat.bit.is_unread_buf = 1;
                result_stat.bit.is_error = 0;
            } else {
                result_stat.bit.is_unread_buf = 0;
                result_stat.bit.is_error = 1;
            }
            break;
        case 0x03:
            DP("Send Data: num_sec=%d\n", num_sec);
            send_sector_data(1, num_sec, buf);
            result_stat.bit.is_unread_buf = 0;
            break;
        case 0x04: {
            int num_sec = receive_dat(1);
            int src_drive = receive_dat(1);
            int src_tr = receive_dat(1);
            int src_sec = receive_dat(1) - 1; // Translate sector number.
            int dst_drive = receive_dat(1);
            int dst_tr = receive_dat(1);
            int dst_sec = receive_dat(1) - 1; // Translate sector number.
            DP("Copy: num_sec=%d (drive=%d,tr=%d,sec=%d)->(drive=%d,trt=%d,sec=%d)\n", num_sec, src_drive, src_tr, src_sec + 1, dst_drive, dst_tr, dst_sec + 1);
            for (int i = 0; i < num_sec; i++) {
                if (md_read(src_drive, src_tr, src_sec, num_sec, buf) != 0) {
                    result_stat.bit.is_error = 1;
                    break;
                }
                if (!md_write(dst_drive, dst_tr, dst_sec, num_sec, buf) != 0) {
                    result_stat.bit.is_error = 1;
                    break;
                }
            }
            result_stat.bit.is_error = 0;
            result_stat.bit.is_unread_buf = 0;
        } break;
        case 0x05:
            drive = receive_dat(1);
            DP("Format: drive=%d\n", drive);
            if (md_format(drive)) {
                result_stat.bit.is_error = 0;
            } else {
                result_stat.bit.is_error = 1;
            }
            break;
        case 0x06:
            DP("Result Status: complete=%d unread=%d Err=%d\n", result_stat.bit.is_io_complete, result_stat.bit.is_unread_buf, result_stat.bit.is_error);
            send_dat(1, (uint16_t)result_stat.dat);
            break;
        case 0x07:
            DP("Drive Status\n");
            send_dat(1, drive_stat);
            break;
        case 0x0b:
            DP("Send Memory Data\n");
            {
                uint8_t addr_H = receive_dat(1);
                uint8_t addr_L = receive_dat(1);
                uint8_t len_H = receive_dat(1);
                uint8_t len_L = receive_dat(1);
                uint16_t addr = (addr_H << 8) | addr_L;
                uint16_t len = (len_H << 8) | len_L;
                DP("Addr=0x%04x, len=0x%04x\n", addr, len);
                uint8_t d = 0x00;
                if (addr == 0x7ef) {
                    d = 0xe0; // EXTON
                }
                DP("\tReturn %02x\n", d);
                send_dat(1, d);
            }
            break;
        case 0x11:
            num_sec = receive_dat(1);
            drive = receive_dat(1);
            tr = receive_dat(1);
            sec = receive_dat(1) - 1; // Translate sector number.;
            DP("Fast Write Disk: num_sec=%d drive=%d tr=%d sec=%d\n", num_sec, drive, tr, sec + 1);
            receive_sector_data(2, num_sec, buf);
            if (md_write(drive, tr, sec, num_sec, buf) == 0) {
                result_stat.bit.is_error = 0;
            } else {
                result_stat.bit.is_error = 1;
            }
            result_stat.bit.is_io_complete = 1;
            break;
        case 0x12:
            DP("Fast Send Data: num_sec=%d\n", num_sec);
            send_sector_data(2, num_sec, buf);
            result_stat.bit.is_unread_buf = 0;
            break;
        case 0x14:
            // Bit7 ESIG: error
            // Bit6 WPDR: write protection
            // Bit5 RDY: ready
            // Bit4 TRK0: track 0
            // Bit3 DSDR: double sided drive
            // Bit2 HDDR: head
            // Bir1:0 DS1,2: drive select
            {
                uint8_t tgt_drv = receive_dat(1);
                uint8_t d = 00101000 | (md_hdr[tgt_drv].write_protect & 1) << 6 | (tr == 0) << 4 | (tr % 2) << 2 | tgt_drv & 0b11;
                DP("Device Status: %02x (tgt=%d WriteProtect=%d)\n", d, tgt_drv, md_hdr[tgt_drv].write_protect);
                send_dat(1, drive_stat);
            }
            break;
        case 0x17: {
            uint8_t m = receive_dat(1);
            DP("Mode Change: %d,%d,%d,%d\n", BIT(m, 3), BIT(m, 2), BIT(m, 1), BIT(m, 0));
        } break;
        default:
            DP("[Undefined]\n");
            break;
        }
    }
    finalize();
}

void finalize() {
    puts("Finalizing...");
    MD_Quit();
    if (gpio) {
        puts("Set All GPIOs to INPUT mode.");
        for (int i = GPIO_NUM_MIN; i <= GPIO_NUM_MAX; i++) {
            func_sel(i, FUNC_INPUT);
        }
    }
    puts("Finished.");
}
