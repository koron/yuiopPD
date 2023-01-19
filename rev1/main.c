#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <tusb.h>

#include "pmw3360.h"
#include "pmw3360_rp2040.h"
#include "usb_descriptors.h"

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

typedef struct {
    uint pin;
    bool pressed;
    uint64_t changed_at;
    int mouse_button;
} direct_button_t;

static direct_button_t direct_buttons[] = {
    { .pin = 26, .pressed = false, .changed_at = 0, .mouse_button =  0 },
    { .pin = 27, .pressed = false, .changed_at = 0, .mouse_button =  1 },
    { .pin = 28, .pressed = false, .changed_at = 0, .mouse_button = -1 },
    { .pin = 29, .pressed = false, .changed_at = 0, .mouse_button =  4 },
    { .pin =  6, .pressed = false, .changed_at = 0, .mouse_button =  3 },
    { .pin =  7, .pressed = false, .changed_at = 0, .mouse_button = -1 },
};

static int16_t mouse_x = 0;
static int16_t mouse_y = 0;
static int8_t mouse_last_btns = false;

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
        direct_button_t b = direct_buttons[i];
        if (b.pressed && b.mouse_button >= 0) {
            btns |= 1 << b.mouse_button;
        }
    }

    if (mouse_x >= 127) {
        mouse_x -= 127;
        x = 127;
    } else if (mouse_x <= -127) {
        mouse_x += 127;
        x = -127;
    } else {
        x = mouse_x;
        mouse_x = 0;
    }

    if (mouse_y >= 127) {
        mouse_y -= 127;
        y = 127;
    } else if (mouse_y <= -127) {
        mouse_y += 127;
        y = -127;
    } else {
        y = mouse_y;
        mouse_y = 0;
    }

    if (x != 0 || y != 0 || v != 0 || h != 0 || btns != mouse_last_btns) {
        tud_hid_n_mouse_report(ITF_NUM_HID, REPORT_ID_MOUSE, btns, x, y, v, h);
        mouse_last_btns = btns;
    }
}

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
        mouse_x += mot.dy;
        mouse_y += mot.dx;
        xiao_led_set_all(false, true, false);
    } else {
        xiao_led_set_all(false, false, true);
    }
}

static void direct_button_task(uint64_t now) {
    // Read direct pins as button.
    uint32_t status = gpio_get_all();
    // Debounce buttons.
    for (int i = 0; i < count_of(direct_buttons); i++) {
        bool curr = (status & (1 << direct_buttons[i].pin)) == 0;
        if (curr != direct_buttons[i].pressed && (now - direct_buttons[i].changed_at) > 10*1000) {
            direct_buttons[i].pressed = curr;
            direct_buttons[i].changed_at = now;
        }
    }
}

int main() {
    xiao_led_init();
    xiao_led_set_all(false, true, false);
    stdio_init_all();
    printf("\nYUIOP/PD rev.1 starting...\n");
    tusb_init();

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

    // Initialize pins for button.
    for (int i = 0; i < count_of(direct_buttons); i++) {
        uint pin = direct_buttons[i].pin;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
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

    while(true) {
        uint64_t now = time_us_64();
        trackball_task(now, &ball);
        direct_button_task(now);
        report_mouse(now);

        tud_task();
    }
}
