#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

#include "pmw3360.h"

static inline void cs_select(pmw3360_inst_t *p) {
    //asm volatile("nop \n nop \n nop");
    gpio_put(p->csn, 0);
    //asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(pmw3360_inst_t *p) {
    //asm volatile("nop \n nop \n nop");
    gpio_put(p->csn, 1);
    //asm volatile("nop \n nop \n nop");
}

uint8_t pmw3360_reg_read(pmw3360_inst_t *p, uint8_t addr) {
    cs_select(p);
    addr &= 0x7f;
    spi_write_blocking(p->spi, &addr, 1);
    busy_wait_us_32(160);
    uint8_t data = 0;
    spi_read_blocking(p->spi, 0x00, &data, 1);
    cs_deselect(p);
    busy_wait_us_32(20);
    return data;
}

void pmw3360_reg_write(pmw3360_inst_t *p, uint8_t addr, uint8_t data) {
    cs_select(p);
    uint8_t buf[2] = { addr | 0x80, data };
    spi_write_blocking(p->spi, buf, 2);
    cs_deselect(p);
    busy_wait_us_32(180);
}

void pmw3360_init(pmw3360_inst_t *p, spi_inst_t *spi, uint csn) {
    p->spi = spi;
    p->csn = csn;
    spi_set_format(p->spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

bool pmw3360_powerup_reset(pmw3360_inst_t *p) {
    pmw3360_reg_write(p, PMW3360_REGADDR_POWER_UP_RESET, 0x5a);
    busy_wait_ms(50);
    pmw3360_reg_read(p, PMW3360_REGADDR_MOTION);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_X_L);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_X_H);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_Y_L);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_Y_H);
    uint8_t pid = pmw3360_reg_read(p, PMW3360_REGADDR_PRODUCT_ID);
    uint8_t rev = pmw3360_reg_read(p, PMW3360_REGADDR_REVISION_ID);
    return pid = 0x42 && rev == 0x01;
}

// read_motion_burst try to read motion burst data, returns false if motion
// burst is not started.
static bool read_motion_burst(pmw3360_inst_t *p, uint8_t *buf12) {
    cs_select(p);
    uint8_t addr = PMW3360_REGADDR_MOTION_BURST;
    spi_write_blocking(p->spi, &addr, 1);
    busy_wait_us_32(35);
    spi_read_blocking(p->spi, 0x00, buf12, 12);
    cs_deselect(p);
    // Required NCS in 500ns after motion burst.
    busy_wait_us_32(1);
    // Check 5th bit "1" when motion burst enabled correctly.
    return (buf12[0] & 0x20) != 0;
}

// reand motion burst data, return true if valid (non zero) motion.
bool pmw3360_read_motion_burst(pmw3360_inst_t *p, pmw3360_motion_t *out) {
    uint8_t buf[12];
    if (!read_motion_burst(p, buf)) {
        // Auto motion burst. Setup motion burst if motion burst is not
        // started.
        pmw3360_reg_write(p, PMW3360_REGADDR_MOTION_BURST, 0);
        if (!read_motion_burst(p, buf)) {
            printf("pmw3360_read_motion_burst: auto burst failed\n");
            return false;
        }
        printf("pmw3360_read_motion_burst: auto burst setup\n");
    }

#if 1
    // DEBUG: log motion burst data each seconds.
    static uint64_t last = 0;
    uint64_t now = time_us_64();
    if (now - last > 1000000) {
        last = now;
        printf("motion_burst capture: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
    }
#endif

    // Parse motion burst data.
    out->mot = buf[0];
    out->obs = buf[1];
    out->dx = buf[2] | (buf[3] << 8);
    out->dy = buf[4] | (buf[5] << 8);

    return out->dx != 0 || out->dy != 0;
}
