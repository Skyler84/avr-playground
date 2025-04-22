#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "module.h"

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

typedef fstatus_t        (*vfs_mount_fn_t)   (void *fs, bool readonly, bool mkfs);
typedef void             (*vfs_umount_fn_t)  (void *fs);
typedef fstatus_t        (*vfs_stat_fn_t)    (void *fs, const char *filename, struct FileInfo *st);
typedef file_descriptor_t(*vfs_open_fn_t)    (void *fs, const char *filename, uint8_t mode);
typedef void             (*vfs_close_fn_t)   (void *fs, file_descriptor_t fd);
typedef fstatus_t        (*vfs_seek_fn_t)    (void *fs, file_descriptor_t fd, uint32_t offset);
typedef fstatus_t        (*vfs_read_fn_t)    (void *fs, file_descriptor_t fd, uint8_t *buf, uint16_t size);
typedef fstatus_t        (*vfs_write_fn_t)   (void *fs, file_descriptor_t fd, const uint8_t *buf, uint16_t size);
typedef fstatus_t        (*vfs_unlink_fn_t)  (void *fs, const char *filename);
typedef fstatus_t        (*vfs_rename_fn_t)  (void *fs, const char *oldname, const char *newname);
typedef file_descriptor_t(*vfs_opendir_fn_t) (void *fs, const char *dirname);
typedef void             (*vfs_closedir_fn_t)(void *fs, file_descriptor_t fd);
typedef fstatus_t        (*vfs_readdir_fn_t) (void *fs, file_descriptor_t fd, struct FileInfo *entry);
typedef void             (*vfs_mkdir_fn_t)   (void *fs, const char *dirname);
typedef void             (*vfs_rmdir_fn_t)   (void *fs, const char *dirname);

typedef struct {
  vfs_mount_fn_t mount;
  vfs_umount_fn_t umount;
  vfs_stat_fn_t stat;
  vfs_open_fn_t open;
  vfs_close_fn_t close;
  vfs_seek_fn_t seek;
  vfs_read_fn_t read;
  vfs_write_fn_t write;
  vfs_unlink_fn_t unlink;
  vfs_rename_fn_t rename;
  vfs_opendir_fn_t opendir;
  vfs_closedir_fn_t closedir;
  vfs_readdir_fn_t readdir;
  vfs_mkdir_fn_t mkdir;
  vfs_rmdir_fn_t rmdir;

} FileSystem_fns_t;

typedef struct {
  FileSystem_fns_t *fns;
} FileSystem_t;

typedef struct {
  FileSystem_t *fs[1];
  struct {
    uint8_t fs_num;
    file_descriptor_t fd;
  } file_descriptor_mappings[16];
}VFS_t;

int16_t mount(VFS_t *vfs, FileSystem_t *fs, const char *path);
int16_t unmount(VFS_t *vfs, FileSystem_t *fs);
file_descriptor_t open(const char *filename, uint8_t mode);
void close(file_descriptor_t fd);
int16_t read(file_descriptor_t fd, uint8_t *buf, uint16_t size);
int16_t write(file_descriptor_t fd, const uint8_t *buf, uint16_t size);
int16_t seek(file_descriptor_t fd, uint32_t offset);
int16_t tell(file_descriptor_t fd, uint32_t *offset);
int16_t remove(const char *filename);
int16_t rename(const char *oldname, const char *newname);
int16_t mkdir(const char *dirname);
int16_t rmdir(const char *dirname);
int16_t stat(const char *filename, struct FileInfo *st);
file_descriptor_t opendir(const char *dirname);
int16_t closedir();
int16_t readdir(file_descriptor_t fd, struct FileInfo *entry);

