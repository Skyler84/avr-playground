#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef int8_t file_descriptor_t;
typedef int16_t fstatus_t;

typedef struct {
  void *_data;
}File;

typedef struct FileInfo {
  uint32_t size;
  uint32_t inode;
  uint8_t type;
  uint8_t attr;
  char name[256];
} FileInfo_t;

typedef fstatus_t        (*fs_mount_fn_t)   (void *fs, bool readonly, bool mkfs);
typedef void             (*fs_umount_fn_t)  (void *fs);
typedef fstatus_t        (*fs_stat_fn_t)    (void *fs, const char *filename, struct FileInfo *st);
typedef file_descriptor_t(*fs_open_fn_t)    (void *fs, const char *filename, uint8_t mode);
typedef file_descriptor_t(*fs_openat_fn_t)  (void *fs, file_descriptor_t, const char *filename, uint8_t mode);
typedef void             (*fs_close_fn_t)   (void *fs, file_descriptor_t fd);
typedef fstatus_t        (*fs_seek_fn_t)    (void *fs, file_descriptor_t fd, uint32_t offset);
typedef fstatus_t        (*fs_read_fn_t)    (void *fs, file_descriptor_t fd, uint8_t *buf, uint16_t size);
typedef fstatus_t        (*fs_write_fn_t)   (void *fs, file_descriptor_t fd, const uint8_t *buf, uint16_t size);
typedef fstatus_t        (*fs_unlink_fn_t)  (void *fs, const char *filename);
typedef fstatus_t        (*fs_rename_fn_t)  (void *fs, const char *oldname, const char *newname);
// typedef file_descriptor_t(*fs_opendir_fn_t) (void *fs, const char *dirname);
// typedef void             (*fs_closedir_fn_t)(void *fs, file_descriptor_t fd);
// typedef fstatus_t        (*fs_readdir_fn_t) (void *fs, file_descriptor_t fd, struct FileInfo *entry);
typedef void             (*fs_mkdir_fn_t)   (void *fs, const char *dirname);
typedef void             (*fs_rmdir_fn_t)   (void *fs, const char *dirname);

typedef struct {
  fs_mount_fn_t mount;
  fs_umount_fn_t umount;
  fs_stat_fn_t stat;
  fs_open_fn_t open;
  fs_close_fn_t close;
  fs_seek_fn_t seek;
  fs_read_fn_t read;
  fs_write_fn_t write;
  fs_unlink_fn_t unlink;
  fs_rename_fn_t rename;
//   fs_opendir_fn_t opendir;
//   fs_closedir_fn_t closedir;
//   fs_readdir_fn_t readdir;
  fs_mkdir_fn_t mkdir;
  fs_rmdir_fn_t rmdir;

} FileSystem_fns_t;

typedef struct {
  FileSystem_fns_t *fns;
} FileSystem_t;