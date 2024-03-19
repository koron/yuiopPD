#pragma once

#include <stdint.h>

void heartbeat_init(uint64_t interval);

void heartbeat_task(uint64_t now);
