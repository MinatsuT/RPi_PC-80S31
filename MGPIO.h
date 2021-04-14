//
// Minatsu GPIO Library by Minatsu
// 7-Apr-2021
//
#ifndef __MGPIO_H_
#define __MGPIO_H_

#include <bcm_host.h>

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/mman.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_REG_OFFSET (0x200000)

volatile uint32_t *gpio = NULL;
#define REG(offset) gpio[(offset) >> 2]

#define MASK(n) (0xffffffff >> (32 - n))
#define BIT(a, b) (((a) >> (b)) & 1)
#define BITS(dat, from, width) ((dat >> from) & MASK(width))
#define BIT_SET(a, b) (a) |= (1 << (b))
#define BIT_CLR(a, b) (a) &= ~(1 << (b))

//================================================================================
// Debug support facilities
//================================================================================
#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#define DP(...) printf(__VA_ARGS__)
#else
#define DP(...)
#endif

// Prototypes
//--------------------------------------------------------------------------------
void finalize(void); // This must be defined at .c file which includes this header.

//================================================================================
// Function select
//================================================================================
#define GPIO_NUM_MIN 2
#define GPIO_NUM_MAX 27
#define FUNC_REG (0x00 >> 2)
#define FUNC_INPUT 0b000
#define FUNC_OUTPUT 0b001
void func_sel(uint32_t gpio_num, uint32_t fn) {
    assert(gpio_num >= GPIO_NUM_MIN && gpio_num <= GPIO_NUM_MAX);
    assert(fn <= 0b111);

    volatile uint32_t *reg = &gpio[FUNC_REG + gpio_num / 10];
    int bit_pos = (gpio_num % 10) * 3;

    *reg &= ~(0b111 << bit_pos);
    *reg |= fn << bit_pos;
}

//================================================================================
// PULL-UP/DOWN
//================================================================================
#define PUD_REG (0xe4 >> 2)
#define PULL_NO 0b00
#define PULL_UP 0b01
#define PULL_DOWN 0b10

// Set PULL-UP/DOWN
void set_pud(uint32_t gpio_num, uint32_t pud) {
    assert(gpio_num >= GPIO_NUM_MIN && gpio_num <= GPIO_NUM_MAX);
    assert(pud <= 0b10);

    volatile uint32_t *reg = &gpio[PUD_REG + gpio_num / 16];
    int bit_pos = (gpio_num % 16) * 2;

    *reg &= ~(0b11 << bit_pos);
    *reg |= pud << bit_pos;
}

//================================================================================
// Read from GPIO
//================================================================================
#define READ_REG (0x34 >> 2)
static inline uint32_t gpio_read(uint32_t from, uint32_t width) { // [from +: width]
    assert(from < 32 && (from + width) <= 32);
    uint32_t d = gpio[READ_REG];
    return BITS(d, from, width);
}

//================================================================================
// Write to GPIO
//================================================================================
#define CLR_REG (0x28 >> 2)
#define SET_REG (0x1c >> 2)
static inline void gpio_write(uint32_t bit_pos, uint32_t val, uint32_t width) {
    assert(bit_pos <= 31 && bit_pos + width <= 32);
    uint32_t mask = MASK(width) << bit_pos;
    gpio[CLR_REG] = mask;
    gpio[SET_REG] = (val << bit_pos) & mask;
}

//================================================================================
// Initialize
//================================================================================
// Signal handler
volatile sig_atomic_t main_run_flag = 1;
void sigintHandler(int sig) {
    puts("Caught signal.");
    finalize();
    exit(0);
}

// Initializer
int MGPIO_Init() {
    int page_size = getpagesize();
    if (page_size < 0) {
        perror("Cannot get pagesize.");
        return -1;
    }

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("cannot open /dev/mem");
        return -1;
    }

    int gpio_base = bcm_host_get_peripheral_address() + GPIO_REG_OFFSET;
    printf("gpio_base=%p\n", (void *)gpio_base);
    void *map = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base);
    close(fd);

    if (map == MAP_FAILED) {
        perror("mmap failed.");
        return -1;
    }
    gpio = (volatile uint32_t *)map;

    // Adding signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("Failed to add signal handler");
        return 1;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // __MGPIO_H_
