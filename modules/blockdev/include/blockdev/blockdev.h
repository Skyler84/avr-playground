#pragma once
#include <stdint.h>

#include "module/module.h"

typedef uint32_t sector_addr_t;
typedef uint8_t status_t;
typedef status_t (*bd_read_sector_fn_t)(void *, sector_addr_t, uint8_t*);
typedef status_t (*bd_write_sector_fn_t)(void *, sector_addr_t, const uint8_t*);
typedef uint32_t (*bd_get_sector_size_fn_t)(void *);
typedef enum {
    BD_ERR_OK = 0,
    BD_ERR_INVALID = 1,
} bd_err_t;

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

#define _(...)
#define BLOCKDEV_FUNCTION_EXPORTS(modname, o) \
    o(modname, partition, status_t, const BlockDev*, BlockDev*, uint32_t, uint32_t) \
    _(modname, get_sector_size, uint32_t, const BlockDev*) \
    o(modname, read_sector, status_t, const BlockDev*, uint32_t, uint8_t*) \
    o(modname, write_sector, status_t, const BlockDev*, uint32_t, const uint8_t*)

#define BLOCKDEV_API_VER 1
#define BLOCKDEV_MODULE_ID 0x0104

DECLARE_MODULE(blockdev, BLOCKDEV_MODULE_ID, BLOCKDEV_FUNCTION_EXPORTS);