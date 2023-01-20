#pragma once

// Interface ID
enum {
    ITF_NUM_HID     = 0,
    ITF_NUM_CDC     = 1, // CDC requires 2 interfaces.
    ITF_NUM_TOTAL   = 3,
};

// HID report ID
enum {
    REPORT_ID_MOUSE  = 1,
};
