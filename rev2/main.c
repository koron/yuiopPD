#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <tusb.h>

#include "direct_button.h"
#include "pmw3360.h"
#include "xiao/led.h"
#include "usb/mouse.h"
#include "heartbeat.h"

#include "usb_descriptors.h"

bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_PIN, "NeoPixel DATA"));
bi_decl(bi_1pin_with_name(PICO_DEFAULT_WS2812_POWER_PIN, "NeoPixel power"));

bi_decl(bi_program_feature("USB Mouse"));

static direct_button_t direct_buttons[] = {
    { .pin = 26, .pressed = false, .changed_at = 0 },
    { .pin = 27, .pressed = false, .changed_at = 0 },
    { .pin = 28, .pressed = false, .changed_at = 0 },
    { .pin = 29, .pressed = false, .changed_at = 0 },
    //{ .pin =  0, .pressed = false, .changed_at = 0 },
};

static int button_actions[] = { -1, 0, 3, 1, 4 };

typedef enum {
    TRACKBALL_MODE_DISABLED = 0,
    TRACKBALL_MODE_POINTER  = 1,
    TRACKBALL_MODE_SCROLL   = 2,
} trackball_mode_t;

static trackball_mode_t trackball_mode = TRACKBALL_MODE_POINTER;

static void trackball_task(uint64_t now, pmw3360_inst_t *ball) {
    // Limitate scan rate: 1000/sec
    static uint64_t last = 0;
    if ((now - last) < 1000) {
        return;
    }
    last = now;

    // Read motion data from PMW3360 optical sensor, and accumulate it.
    pmw3360_motion_t mot = {0};
    if (pmw3360_read_motion_burst(ball, &mot)) {
        switch (trackball_mode) {
            case TRACKBALL_MODE_POINTER:
                usb_mouse_xy_append(mot.dy, mot.dx);
                break;
            case TRACKBALL_MODE_SCROLL:
                usb_mouse_vh_append(-mot.dx, 0);
                break;
        }
        xiao_led_set_all(false, true, false);
    } else {
        xiao_led_set_all(false, false, true);
    }
}

void direct_button_on_changed(uint64_t now, int num, bool pressed) {
    printf("direct_button_on_changed: num=%d pressed=%s now=%llu\n", num, pressed ? "true" : "false", now);

#if 0
    if (num == 4) {
        // Toggle trackball enable/disable
        if (pressed) {
            trackball_mode = trackball_mode != TRACKBALL_MODE_DISABLED ? TRACKBALL_MODE_DISABLED : TRACKBALL_MODE_POINTER;
        }
        return;
    }
#endif

    if (num == 0) {
        // Scroll mode when pressed.
        if (trackball_mode != TRACKBALL_MODE_DISABLED) {
            trackball_mode = pressed ? TRACKBALL_MODE_SCROLL : TRACKBALL_MODE_POINTER;
        }
        usb_mouse_xy_reset();
        usb_mouse_vh_reset();
        return;
    }

    int ba = button_actions[num];
    if (ba >= 0) {
        usb_mouse_set_button((uint8_t)ba, pressed);
    }
}

int main() {
    xiao_led_init();
    xiao_led_set_all(false, true, false);
    heartbeat_init(5*1000*1000);
    tusb_init();
    stdio_init_all();
    printf("\nYUIOP/PD rev.2 starting...\n");

    // Initialize buttons.
    {
        direct_button_init(direct_buttons, count_of(direct_buttons));
        bi_decl(bi_pin_mask_with_name(0x3c000001, "Buttons"));
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
    pmw3360_init(&ball, spi0, PICO_DEFAULT_SPI_CSN_PIN);
    bool ball_ok = pmw3360_powerup_reset(&ball);
    if (!ball_ok) {
        xiao_led_set_all(true, false, false);
        printf("failed to ball initialization\n");
        return -1;
    }
    printf("ball on SPI0 is initialized\n");

    pmw3360_srom_upload(&ball, pmw3360_srom_rev4);

    // Set 2000 CPI (DPI)
    pmw3360_set_cpi(&ball, 20);

    // Setup USB mouse.
    usb_mouse_init(ITF_NUM_HID, REPORT_ID_MOUSE);
    usb_mouse_set_div_v(5);

    // Start main loop.
    xiao_led_set_all(false, false, true);

    uint64_t last_sec = 0;

    while(true) {
        uint64_t now = time_us_64();
        heartbeat_task(now);
        direct_button_task(now);
        trackball_task(now, &ball);
        usb_mouse_task(now);
        tud_task();
    }
}
