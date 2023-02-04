#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <tusb.h>

#include "direct_button.h"
#include "pmw3360.h"
#include "pmw3360_rp2040.h"
#include "xiao/led.h"

#include "usb_descriptors.h"

bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_PIN, "NeoPixel DATA"));
bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_POWER_PIN, "NeoPixel power"));

bi_decl(bi_program_feature("USB Mouse"));

static direct_button_t direct_buttons[] = {
    { .pin = 26, .pressed = false, .changed_at = 0 },
    { .pin = 27, .pressed = false, .changed_at = 0 },
    { .pin = 28, .pressed = false, .changed_at = 0 },
    { .pin = 29, .pressed = false, .changed_at = 0 },
    { .pin =  6, .pressed = false, .changed_at = 0 },
    { .pin =  7, .pressed = false, .changed_at = 0 },
};

static int button_actions[] = { 0, 1, -1, 4, 3, -1 };

static int16_t mouse_x = 0;
static int16_t mouse_y = 0;
static int8_t mouse_last_btns = false;

// add16 adds two int16_t with clipping.
static int16_t add16(int16_t a, int16_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 32767;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -32768;
    }
    return r;
}

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

static int mouse_mode = 0;

const static int DIV = 5;

static void report_mouse(uint64_t now) {
    if (!tud_hid_n_ready(ITF_NUM_HID)) {
        return;
    }

    uint8_t btns = 0;
    int8_t x = 0;
    int8_t y = 0;
    int8_t v = 0;
    int8_t h = 0;

    for (int i = 0; i < count_of(direct_buttons); i++) {
        if (direct_buttons[i].pressed && button_actions[i] >= 0) {
            btns |= 1 << button_actions[i];
        }
    }

    switch (mouse_mode) {
        case 1:
            v = (clip2int8(mouse_y) >> DIV);
            mouse_x = 0;
            mouse_y -= v << DIV;
            v = -v;
            break;

        default:
            x = clip2int8(mouse_x);
            y = clip2int8(mouse_y);
            mouse_x -= x;
            mouse_y -= y;
            break;
    }

    if (x != 0 || y != 0 || v != 0 || h != 0 || btns != mouse_last_btns) {
        tud_hid_n_mouse_report(ITF_NUM_HID, REPORT_ID_MOUSE, btns, x, y, v, h);
        mouse_last_btns = btns;
    }
}

static bool trackball_enabled = true;

static void trackball_task(uint64_t now, pmw3360_inst_t *ball) {
    // Limitate scan rate: 1000/sec
    static uint64_t last = 0;
    if ((now - last) < 1000) {
        return;
    }
    last = now;

#if 1
    // PMW3360's motion burst fails without writing the register sometimes.
    pmw3360_reg_write(ball, PMW3360_REGADDR_MOTION_BURST, 0);
#endif

    // Read motion data from PMW3360 optical sensor, and accumulate it.
    pmw3360_motion_t mot = {0};
    if (pmw3360_read_motion_burst(ball, &mot)) {
        if (trackball_enabled) {
            mouse_x = add16(mouse_x, mot.dy);
            mouse_y = add16(mouse_y, mot.dx);
            xiao_led_set_all(false, true, false);
        }
    } else {
        xiao_led_set_all(false, false, true);
    }
}

void direct_button_on_changed(uint64_t now, int num, bool pressed) {
    printf("direct_button_on_changed: num=%d pressed=%s now=%llu\n", num, pressed ? "true" : "false", now);

    if (num == 2) {
        // Toggle trackball enable/disable
        if (pressed) {
            trackball_enabled = !trackball_enabled;
            printf("  trackball_enabled=%s\n", trackball_enabled ? "true" : "false");
        }
        return;
    }

    if (num == 5) {
        // Scroll mode when pressed.
        mouse_mode = pressed ? 1 : 0;
        mouse_x = 0;
        mouse_y = 0;
        return;
    }
}

int main() {
    xiao_led_init();
    xiao_led_set_all(false, true, false);
    tusb_init();
    stdio_init_all();
    printf("\nYUIOP/PD rev.1 starting...\n");

    // Initialize buttons.
    {
        direct_button_init(direct_buttons, count_of(direct_buttons));
        bi_decl(bi_pin_mask_with_name(0x3c0000c0, "Buttons"));
    }

    // Initialize trackball module.
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

    // Start main loop.
    xiao_led_set_all(false, false, true);

    uint64_t last_sec = 0;

    while(true) {
        uint64_t now = time_us_64();
        trackball_task(now, &ball);
        direct_button_task(now);
        report_mouse(now);

        uint64_t sec = now / (1000 * 1000);
        if (sec != last_sec && (sec % 5) == 0) {
            printf("YUIOP/PD: heartbeat at %llu seconds\n", sec);
            last_sec = sec;
        }

        tud_task();
    }
}
