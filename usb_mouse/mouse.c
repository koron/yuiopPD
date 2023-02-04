#include <stdio.h>

#include "tusb.h"

#include "usb/mouse.h"

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

static inline int8_t consume_int8(int16_t *p, uint8_t div) {
    int8_t v8 = clip2int8(*p) >> div;
    *p = *p - (v8 << div);
    return v8;
}

static uint8_t instance = 0;
static uint8_t report_id = 0;

static uint8_t buttons = 0;
static int16_t accum_x = 0;
static int16_t accum_y = 0;
static int16_t accum_v = 0;
static int16_t accum_h = 0;

static uint8_t div_x = 0;
static uint8_t div_y = 0;
static uint8_t div_v = 0;
static uint8_t div_h = 0;

void usb_mouse_init(uint8_t mouse_instance, uint8_t mouse_report_id) {
    instance = mouse_instance;
    report_id = mouse_report_id;
}

void usb_mouse_task(uint64_t now) {
    if (!tud_hid_n_ready(instance)) {
        return;
    }
    static uint8_t last_buttons = 0;
    int8_t x = consume_int8(&accum_x, div_x);
    int8_t y = consume_int8(&accum_y, div_y);
    int8_t v = consume_int8(&accum_v, div_v);
    int8_t h = consume_int8(&accum_h, div_h);
    if (buttons != last_buttons || x != 0 || y != 0 || v != 0 || h != 0) {
        last_buttons = buttons;
        tud_hid_n_mouse_report(instance, report_id, buttons, x, y, v, h);
    }
}

bool usb_mouse_get_button(uint8_t n) {
    return buttons & (1 << n);
}

void usb_mouse_set_button(uint8_t n, bool pressed) {
    uint8_t mask = 1 << n;
    if (pressed) {
        buttons |= mask;
    } else {
        buttons &= ~mask;
    }
}

uint8_t usb_mouse_get_button_all(void) {
    return buttons;
}

void usb_mouse_set_button_all(uint8_t v) {
    buttons = v;
}

void usb_mouse_xy_append(int16_t dx, int16_t dy) {
    accum_x = add16(accum_x, dx);
    accum_y = add16(accum_y, dy);
}

void usb_mouse_xy_reset(void) {
    accum_x = 0;
    accum_y = 0;
}

void usb_mouse_vh_append(int16_t dv, int16_t dh) {
    accum_v = add16(accum_v, dv);
    accum_h = add16(accum_h, dh);
}

void usb_mouse_vh_reset(void) {
    accum_v = 0;
    accum_h = 0;
}

void usb_mouse_set_div_x(uint8_t v) {
    div_x = v;
}

uint8_t usb_mouse_get_div_x(void) {
    return div_x;
}

void    usb_mouse_set_div_y(uint8_t v) {
    div_y = v;
}

uint8_t usb_mouse_get_div_y(void) {
    return div_y;
}

void    usb_mouse_set_div_v(uint8_t v) {
    div_v = v;
}

uint8_t usb_mouse_get_div_v(void) {
    return div_v;
}

void    usb_mouse_set_div_h(uint8_t v) {
    div_h = v;
}

uint8_t usb_mouse_get_div_h(void) {
    return div_h;
}
