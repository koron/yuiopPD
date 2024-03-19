#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hardware/spi.h"

typedef enum {
    PMW3360_REGADDR_PRODUCT_ID                 = 0x00,
    PMW3360_REGADDR_REVISION_ID                = 0x01,
    PMW3360_REGADDR_MOTION                     = 0x02,
    PMW3360_REGADDR_DELTA_X_L                  = 0x03,
    PMW3360_REGADDR_DELTA_X_H                  = 0x04,
    PMW3360_REGADDR_DELTA_Y_L                  = 0x05,
    PMW3360_REGADDR_DELTA_Y_H                  = 0x06,
    PMW3360_REGADDR_SQUAL                      = 0x07,
    PMW3360_REGADDR_RAW_DATA_SUM               = 0x08,
    PMW3360_REGADDR_MAXIMUM_RAW_DATA           = 0x09,
    PMW3360_REGADDR_MINIMUM_RAW_DATA           = 0x0A,
    PMW3360_REGADDR_SHUTTER_LOWER              = 0x0B,
    PMW3360_REGADDR_SHUTTER_UPPER              = 0x0C,
    PMW3360_REGADDR_CONTROL                    = 0x0D,
    PMW3360_REGADDR_CONFIG1                    = 0x0F,
    PMW3360_REGADDR_CONFIG2                    = 0x10,
    PMW3360_REGADDR_ANGLE_TUNE                 = 0x11,
    PMW3360_REGADDR_FRAME_CAPTURE              = 0x12,
    PMW3360_REGADDR_SROM_ENABLE                = 0x13,
    PMW3360_REGADDR_RUN_DOWNSHIFT              = 0x14,
    PMW3360_REGADDR_REST1_RATE_LOWER           = 0x15,
    PMW3360_REGADDR_REST1_RATE_UPPER           = 0x16,
    PMW3360_REGADDR_REST1_DOWNSHIFT            = 0x17,
    PMW3360_REGADDR_REST2_RATE_LOWER           = 0x18,
    PMW3360_REGADDR_REST2_RATE_UPPER           = 0x19,
    PMW3360_REGADDR_REST2_DOWNSHIFT            = 0x1A,
    PMW3360_REGADDR_REST3_RATE_LOWER           = 0x1B,
    PMW3360_REGADDR_REST3_RATE_UPPER           = 0x1C,
    PMW3360_REGADDR_OBSERVATION                = 0x24,
    PMW3360_REGADDR_DATA_OUT_LOWER             = 0x25,
    PMW3360_REGADDR_DATA_OUT_UPPER             = 0x26,
    PMW3360_REGADDR_RAW_DATA_DUMP              = 0x29,
    PMW3360_REGADDR_SROM_ID                    = 0x2A,
    PMW3360_REGADDR_MIN_SQ_RUN                 = 0x2B,
    PMW3360_REGADDR_RAW_DATA_THRESHOLD         = 0x2C,
    PMW3360_REGADDR_CONFIG5                    = 0x2F,
    PMW3360_REGADDR_POWER_UP_RESET             = 0x3A,
    PMW3360_REGADDR_SHUTDOWN                   = 0x3B,
    PMW3360_REGADDR_INVERSE_PRODUCT_ID         = 0x3F,
    PMW3360_REGADDR_LIFTCUTOFF_TUNE3           = 0x41,
    PMW3360_REGADDR_ANGLE_SNAP                 = 0x42,
    PMW3360_REGADDR_LIFTCUTOFF_TUNE1           = 0x4A,
    PMW3360_REGADDR_MOTION_BURST               = 0x50,
    PMW3360_REGADDR_LIFTCUTOFF_TUNE_TIMEOUT    = 0x58,
    PMW3360_REGADDR_LIFTCUTOFF_TUNE_MIN_LENGTH = 0x5A,
    PMW3360_REGADDR_SROM_LOAD_BURST            = 0x62,
    PMW3360_REGADDR_LIFT_CONFIG                = 0x63,
    PMW3360_REGADDR_RAW_DATA_BURST             = 0x64,
    PMW3360_REGADDR_LIFTCUTOFF_TUNE2           = 0x65,
} pmw3360_regaddr_t;

typedef struct {
    spi_inst_t *spi;
    uint        csn;

    bool motion_bursting;
} pmw3360_inst_t;

void pmw3360_init(pmw3360_inst_t *p, spi_inst_t *spi, uint csn);

uint8_t pmw3360_reg_read(pmw3360_inst_t *p, uint8_t addr);

void pmw3360_reg_write(pmw3360_inst_t *p, uint8_t addr, uint8_t data);

bool pmw3360_powerup_reset(pmw3360_inst_t *p);

typedef struct {
    const uint8_t *data;
    size_t         len;
} pmw3360_srom_t;

extern const pmw3360_srom_t pmw3360_srom_rev4;

void pmw3360_srom_upload(pmw3360_inst_t *p, pmw3360_srom_t srom);

typedef struct {
    uint8_t mot;
    uint8_t obs;
    int16_t dx;
    int16_t dy;
} pmw3360_motion_t;

void pmw3360_enable_motion_burst(pmw3360_inst_t *p);

bool pmw3360_read_motion_burst(pmw3360_inst_t *p, pmw3360_motion_t *out);
