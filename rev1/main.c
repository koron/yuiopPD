#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>

#include "pmw3360.h"
#include "pmw3360_rp2040.h"

int main() {
    stdio_init_all();
    printf("\nYUIOP/PD rev.1 starting...\n");

    {
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
        bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

        gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
        bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        spi_init(spi0, 2*1000*1000);
    }

    pmw3360_inst_t ball;
    pmw3360_init_rp2040(&ball, spi0, PICO_DEFAULT_SPI_CSN_PIN);
    bool ball_ok = pmw3360_powerup_reset(&ball);
    if (!ball_ok) {
        printf("failed to ball initialization\n");
        return -1;
    }
    printf("ball on SPI0 is initialized\n");

    // Set 2000 CPI (DPI)
    pmw3360_reg_write(&ball, PMW3360_REGADDR_CONFIG1, 19);
    // Set motion burst mode.
    pmw3360_reg_write(&ball, PMW3360_REGADDR_MOTION_BURST, 0);

    uint64_t last_scan = 0;
    while(true) {
        uint64_t now = time_us_64();
        if (now - last_scan >= 100*1000) {
            pmw3360_motion_t curr;
            if (pmw3360_read_motion_burst(&ball, &curr)) {
                printf("motion: dx=%d dy=%d at %llu\n", curr.dx, curr.dy, now);
            } else if ((curr.mot & 0x80) == 0x00) {
                // Set motion burst mode, again
                pmw3360_reg_write(&ball, PMW3360_REGADDR_MOTION_BURST, 0);
            }
            last_scan = now;
        }
        tight_loop_contents();
    }
}
