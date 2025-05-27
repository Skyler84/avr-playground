#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "fs/fs.h"
#include "module/module.h"

typedef union VFS VFS_t;

#define VFS_FUNCTIONS_EXPORT(modname, o) \
  FS_FUNCTION_INTERFACE(modname, o) \
  o(modname, mountfs, fstatus_t, VFS_t *vfs, FileSystem_t *fs, const char *path) \
  o(modname, unmountfs, fstatus_t, VFS_t *vfs, FileSystem_t *fs)

#define VFS_MODULE_ID 0x0130
#define VFS_API_VER 1

DECLARE_MODULE(vfs, VFS_MODULE_ID, VFS_FUNCTIONS_EXPORT);

union VFS{
  FileSystem_t base;
  struct{
    vfs_fns_t *fns;
    struct {
      char path[256];
      FileSystem_t *fs;
    } filesystem_mappings[1];
    struct {
      uint8_t fs_num;
      file_descriptor_t fd;
    } file_descriptor_mappings[16];
  };
};