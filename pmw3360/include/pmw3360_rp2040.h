#pragma once

#include "hardware/spi.h"

struct pmw3360_inst {
    spi_inst_t *spi;
    uint        csn;
};

void pmw3360_init_rp2040(pmw3360_inst_t *p, spi_inst_t *spi, uint csn);
