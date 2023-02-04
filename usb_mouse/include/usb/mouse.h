#pragma once

void usb_mouse_init(uint8_t mouse_instance, uint8_t mouse_report_id);

void usb_mouse_task(uint64_t now);

bool usb_mouse_get_button(uint8_t n);

void usb_mouse_set_button(uint8_t n, bool pressed);

uint8_t usb_mouse_get_button_all(void);

void usb_mouse_set_button_all(uint8_t v);

void usb_mouse_xy_append(int16_t dx, int16_t dy);

void usb_mouse_xy_reset(void);

void usb_mouse_vh_append(int16_t dv, int16_t dh);

void usb_mouse_vh_reset(void);

void    usb_mouse_set_div_x(uint8_t v);
uint8_t usb_mouse_get_div_x(void);
void    usb_mouse_set_div_y(uint8_t v);
uint8_t usb_mouse_get_div_y(void);
void    usb_mouse_set_div_v(uint8_t v);
uint8_t usb_mouse_get_div_v(void);
void    usb_mouse_set_div_h(uint8_t v);
uint8_t usb_mouse_get_div_h(void);
