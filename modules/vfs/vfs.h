#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "fs.h"
#include "module.h"

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

