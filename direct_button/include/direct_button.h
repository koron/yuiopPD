#pragma once

typedef struct {
    uint     pin;
    bool     pressed;
    uint64_t changed_at;
} direct_button_t;

void direct_button_init(direct_button_t *buttons, int count);

void direct_button_task(uint64_t now);

void direct_button_on_changed(uint64_t now, int num, bool pressed);
