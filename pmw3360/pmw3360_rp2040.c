#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

#include "pmw3360.h"
#include "pmw3360_rp2040.h"

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

void pmw3360_init_rp2040(pmw3360_inst_t *p, spi_inst_t *spi, uint csn) {
    p->spi = spi;
    p->csn = csn;
    spi_set_format(p->spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

bool pmw3360_powerup_reset(pmw3360_inst_t *p) {
    pmw3360_reg_write(p, PMW3360_REGADDR_POWER_UP_RESET, 0x5a);
    busy_wait_us_32(50);
    pmw3360_reg_read(p, PMW3360_REGADDR_MOTION);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_X_L);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_X_H);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_Y_L);
    pmw3360_reg_read(p, PMW3360_REGADDR_DELTA_Y_H);
    uint8_t pid = pmw3360_reg_read(p, PMW3360_REGADDR_PRODUCT_ID);
    uint8_t rev = pmw3360_reg_read(p, PMW3360_REGADDR_REVISION_ID);
    return pid = 0x42 && rev == 0x01;
}

bool pmw3360_read_motion_burst(pmw3360_inst_t *p, pmw3360_motion_t *out) {
    bool retval = false;
    cs_select(p);

    uint8_t addr = PMW3360_REGADDR_MOTION_BURST;
    spi_write_blocking(p->spi, &addr, 1);
    busy_wait_us_32(35);
    uint8_t mot;
    spi_read_blocking(p->spi, 0x00, &mot, 1);
    if ((mot & 0x88) != 0x80) {
        out->mot = mot;
        goto MOTION_BURST_END;
    }
    uint8_t data[5] = {0};
    spi_read_blocking(p->spi, 0x00, data, 5);
    out->mot = mot;
    out->obs = data[0];
    out->dx = data[1] | (data[2] << 8);
    out->dy = data[3] | (data[4] << 8);
    retval = true;

MOTION_BURST_END:
    cs_deselect(p);
    busy_wait_us_32(1);
    return retval;
}
