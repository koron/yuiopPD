#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>

#include "pmw3360.h"
#include "pmw3360_rp2040.h"

#define XIAO_RED_LED_PIN    17
#define XIAO_GREEN_LED_PIN  16
#define XIAO_BLUE_LED_PIN   25

bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_PIN, "NeoPixel DATA"));
bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_POWER_PIN, "NeoPixel power"));

void xiao_led_init() {
    gpio_init(XIAO_RED_LED_PIN);
    gpio_set_dir(XIAO_RED_LED_PIN, GPIO_OUT);
    gpio_init(XIAO_GREEN_LED_PIN);
    gpio_set_dir(XIAO_GREEN_LED_PIN, GPIO_OUT);
    gpio_init(XIAO_BLUE_LED_PIN);
    gpio_set_dir(XIAO_BLUE_LED_PIN, GPIO_OUT);
    bi_decl(bi_1pin_with_name(XIAO_RED_LED_PIN, "Red LED"));
    bi_decl(bi_1pin_with_name(XIAO_GREEN_LED_PIN, "Green LED"));
    bi_decl(bi_1pin_with_name(XIAO_BLUE_LED_PIN, "Blue LED"));
}

void xiao_led_set_all(bool red, bool green, bool blue) {
    uint32_t mask = (1 << XIAO_RED_LED_PIN) |
        (1 << XIAO_GREEN_LED_PIN) |
        (1 << XIAO_BLUE_LED_PIN);
    uint32_t value = (red ? 0 : (1 << XIAO_RED_LED_PIN)) |
            (green ? 0 : (1 << XIAO_GREEN_LED_PIN)) |
            (blue ? 0 : (1 << XIAO_BLUE_LED_PIN));
    gpio_put_masked(mask, value);
}

bi_decl(bi_program_feature("USB Mouse"));
bi_decl(bi_pin_mask_with_name(0x3c0000c0, "Buttons"));

const uint button_pins[] = { 26, 27, 28, 29, 6, 7 };
static uint32_t button_masks = 0;

int main() {
    xiao_led_init();
    xiao_led_set_all(false, true, false);
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
        bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI0 CS"));

        spi_init(spi0, 2*1000*1000);
    }

    // initialization pins for button.
    uint32_t button_masks = 0;
    for (int i = 0; i < count_of(button_pins); i++) {
        uint pin = button_pins[i];
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
        button_masks |= 1 << pin;
    }

    // Initialize the trackball module.
    pmw3360_inst_t ball;
    pmw3360_init_rp2040(&ball, spi0, PICO_DEFAULT_SPI_CSN_PIN);
    bool ball_ok = pmw3360_powerup_reset(&ball);
    if (!ball_ok) {
        xiao_led_set_all(true, false, false);
        printf("failed to ball initialization\n");
        return -1;
    }
    printf("ball on SPI0 is initialized\n");

    // Set 2000 CPI (DPI)
    pmw3360_reg_write(&ball, PMW3360_REGADDR_CONFIG1, 19);
    // Set motion burst mode.
    pmw3360_reg_write(&ball, PMW3360_REGADDR_MOTION_BURST, 0);

    xiao_led_set_all(false, false, true);

    uint64_t last_scan = 0;
    uint64_t button_states_last = 0;
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

        uint32_t button_states = gpio_get_all() & button_masks ^ button_masks;
        if (button_states != button_states_last) {
            button_states_last = button_states;
            printf("buttons: %08x\n", button_states);
        }

        tight_loop_contents();
    }
}
