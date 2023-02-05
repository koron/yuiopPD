#include <stdio.h>

#include "heartbeat.h"

static uint64_t s_interval = 0;
static uint64_t s_last = 0;

void heartbeat_init(uint64_t interval) {
    s_interval = interval;
}

void heartbeat_task(uint64_t now) {
    if (now - s_last < s_interval) {
        return;
    }
    s_last = now - (now % s_interval);
    printf("heartbeat at %llu\n", now);
}
