#pragma once
#include <stdint.h>

typedef uint32_t sector_addr_t;
typedef uint8_t status_t;
typedef status_t (*bd_read_sector_fn_t)(const void *, sector_addr_t, uint8_t*);
typedef status_t (*bd_write_sector_fn_t)(const void *, sector_addr_t, const uint8_t*);
typedef uint32_t (*bd_get_sector_size_fn_t)(const void *);
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


inline static status_t blockdev_partition(const BlockDev *bd, BlockDev *out, uint32_t sector_start, uint32_t sector_count)
{
  if ((sector_start + sector_count) > bd->sector_count) {
    return BD_ERR_INVALID;
  }
  out->sector_start = bd->sector_start + sector_start;
  out->sector_count = sector_count;
  return BD_ERR_OK;
}

inline static status_t blockdev_read_sector(const BlockDev *bd, uint32_t sector, uint8_t *buf)
{
  if (sector >= bd->sector_count) {
    return BD_ERR_INVALID;
  }
  return bd->fns->read_sector(bd->fn_ctx, sector + bd->sector_start, buf);
}

inline static status_t blockdev_write_sector(const BlockDev *bd, uint32_t sector, const uint8_t *buf)
{
  if (sector >= bd->sector_count) {
    return BD_ERR_INVALID;
  }
  return bd->fns->write_sector(bd->fn_ctx, sector + bd->sector_start, buf);
}


