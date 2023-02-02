#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "xiao/led.h"

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
