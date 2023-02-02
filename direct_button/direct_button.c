#include <stdio.h>

#include "hardware/gpio.h"

#include "direct_button.h"

static direct_button_t *s_buttons = NULL;

static int s_count = 0;

void direct_button_init(direct_button_t *buttons, int count) {
    s_buttons = buttons;
    s_count = count;
    // Initialize pins for button.
    for (int i = 0; i < s_count; i++) {
        uint pin = s_buttons[i].pin;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
}

void direct_button_task(uint64_t now) {
    if (s_buttons == NULL || s_count <= 0) {
        return;
    }
    // Read direct pins as button.
    uint32_t status = gpio_get_all();
    // Debounce buttons.
    for (int i = 0; i < s_count; i++) {
        bool curr = (status & (1 << s_buttons[i].pin)) == 0;
        if (curr != s_buttons[i].pressed && (now - s_buttons[i].changed_at) > 10*1000) {
            s_buttons[i].pressed = curr;
            s_buttons[i].changed_at = now;
            direct_button_on_changed(now, i, curr);
        }
    }
}

__attribute__((weak)) void direct_button_on_changed(uint64_t now, int num, bool pressed) {
    printf("direct_button_on_changed: num=%d pressed=%s now=%llu\n", num, pressed ? "true" : "false", now);
}
