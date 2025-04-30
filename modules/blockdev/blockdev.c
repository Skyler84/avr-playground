#include "blockdev/blockdev.h"

status_t blockdev_partition(const BlockDev *bd, BlockDev *out, uint32_t sector_start, uint32_t sector_count)
{
  if ((sector_start + sector_count) > bd->sector_count) {
    return BD_ERR_INVALID;
  }
  out->sector_start = bd->sector_start + sector_start;
  out->sector_count = sector_count;
}

status_t blockdev_read_sector(const BlockDev *bd, uint32_t sector, uint8_t *buf)
{
  if (sector >= bd->sector_count) {
    return BD_ERR_INVALID;
  }
  return bd->fns->read_sector(bd->fn_ctx, sector + bd->sector_start, buf);
}

status_t blockdev_write_sector(const BlockDev *bd, uint32_t sector, const uint8_t *buf)
{
  if (sector >= bd->sector_count) {
    return BD_ERR_INVALID;
  }
  return bd->fns->write_sector(bd->fn_ctx, sector + bd->sector_start, buf);
}



REGISTER_MODULE(blockdev, BLOCKDEV_MODULE_ID, BLOCKDEV_FUNCTION_EXPORTS, BLOCKDEV_API_VER);