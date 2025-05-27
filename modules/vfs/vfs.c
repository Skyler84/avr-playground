#include "fs/vfs.h"

#include <string.h>

fstatus_t vfs_mountfs(VFS_t *vfs, FileSystem_t *fs, const char *path)
{
  if (strlen(path) >= sizeof(vfs->filesystem_mappings[0].path)) {
    return -1; // path too long
  }
}