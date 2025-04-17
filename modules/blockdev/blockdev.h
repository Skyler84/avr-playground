#pragma once
#include <stdint.h>

#include "module.h"

typedef uint32_t sector_addr_t;
typedef uint8_t status_t;
typedef status_t (*bd_read_sector_fn_t)(const void *, sector_addr_t, uint8_t*);
typedef status_t (*bd_write_sector_fn_t)(const void *, sector_addr_t, const uint8_t*);

typedef struct {
    bd_read_sector_fn_t read_sector;
    bd_write_sector_fn_t write_sector;
} blockdev_sector_fns_t;

typedef struct {
    const blockdev_sector_fns_t *fns;
    void *fn_ctx;
    uint32_t sector_start;
    uint32_t sector_count;
} BlockDev;

extern status_t bd_partition(const BlockDev *bd, BlockDev *out, uint32_t sector_start, uint32_t sector_count);
extern status_t bd_read_sector(const BlockDev *bd, uint32_t sector, uint8_t *buf);
extern status_t bd_write_sector(const BlockDev *bd, uint32_t sector, const uint8_t *buf);