#pragma once

#ifndef XIAO_RED_LED_PIN
#   define XIAO_RED_LED_PIN 17
#endif

#ifndef XIAO_GREEN_LED_PIN
#   define XIAO_GREEN_LED_PIN 16
#endif

#ifndef XIAO_BLUE_LED_PIN
#   define XIAO_BLUE_LED_PIN   25
#endif

void xiao_led_init();

void xiao_led_set_all(bool red, bool green, bool blue);
