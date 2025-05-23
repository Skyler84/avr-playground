#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "module/module.h"

typedef int8_t file_descriptor_t;
typedef int16_t fstatus_t;

#define O_RDONLY  0x01
#define O_WRONLY  0x02
#define O_RDWR    0x03
#define O_DIRECTORY 0x04
#define O_CREAT 0x08

#define fs_MODTYPE MODULE

typedef struct {
  void *_data;
}File;

typedef struct FileInfo {
  uint32_t size;
  uint32_t inode;
  uint8_t type;
  uint8_t attr;
  char name[256];
  char altname[16];
} FileInfo_t;

typedef struct FileSystem FileSystem_t;

#define FS_FUNCTION_INTERFACE(modname, o) \
  o(modname, mount, fstatus_t,           FileSystem_t *fs, bool readonly, bool mkfs) \
  o(modname, umount, void,               FileSystem_t *fs) \
  o(modname, stat, fstatus_t,            FileSystem_t *fs, const char *filename, struct FileInfo *st) \
  o(modname, open, file_descriptor_t,    FileSystem_t *fs, const char *filename, uint8_t mode) \
  o(modname, openat, file_descriptor_t,  FileSystem_t *fs, file_descriptor_t, const char *filename, uint8_t mode) \
  o(modname, close, void,                FileSystem_t *fs, file_descriptor_t fd) \
  o(modname, seek, fstatus_t,            FileSystem_t *fs, file_descriptor_t fd, uint32_t offset, int whence) \
  o(modname, read, fstatus_t,            FileSystem_t *fs, file_descriptor_t fd, char *buf, uint16_t size) \
  o(modname, write, fstatus_t,           FileSystem_t *fs, file_descriptor_t fd, const char *buf, uint16_t size) \
  o(modname, unlink, fstatus_t,          FileSystem_t *fs, const char *filename) \
  o(modname, rename, fstatus_t,          FileSystem_t *fs, const char *oldname, const char *newname) \
  o(modname, mkdir, void,                FileSystem_t *fs, const char *dirname) \
  o(modname, rmdir, void,                FileSystem_t *fs, const char *dirname) \
  o(modname, getdirents, fstatus_t,      FileSystem_t *fs, file_descriptor_t fd, struct FileInfo *entry, uint16_t count) \

MODULE_DECLARE_FN_IDS(fs, FS_FUNCTION_INTERFACE)
MODULE_DECLARE_FN_TYPES(fs, FS_FUNCTION_INTERFACE)
MODULE_DECLARE_FNS(fs, FS_FUNCTION_INTERFACE);
struct FileSystem {
  fs_fns_t *fns;
};