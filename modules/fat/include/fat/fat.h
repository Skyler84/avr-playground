#pragma once

#include "module/module.h"
#include "blockdev/blockdev.h"
#include "fs/fs.h"

#include <stdint.h>


typedef struct BPB2 {
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t num_fats;
  uint16_t root_entries;
  uint16_t total_sectors_16;
  uint8_t media_descriptor;
  uint16_t sectors_per_fat;;
} __attribute__((packed)) BPB2_t;

typedef struct BPB3_0 {
  BPB2_t bpb2;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint16_t hidden_sectors;
} __attribute__((packed)) BPB3_0_t;

typedef struct BPB3_2{
  BPB3_0_t bpb3;
  uint16_t total_sectors;
} __attribute__((packed)) BPB3_2_t;

typedef struct BPB3_31 {
  BPB2_t bpb2;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
} __attribute__((packed)) BPB3_31_t;

typedef struct EBPB {
  BPB3_31_t bpb31;
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  uint8_t volume_label[11];
  uint8_t file_system_type[8];
} __attribute__((packed)) EBPB_t;
typedef struct FAT32EBPB{
  BPB3_31_t bpb31;
  uint32_t sectors_per_fat;
  uint16_t flags;
  uint16_t version;
  uint32_t root_cluster;
  uint16_t fsinfo_sector;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
} __attribute__((packed)) FAT32EBPB_t;

typedef struct Cluster {
  uint32_t sector_start;
  uint32_t sector_count;
  uint32_t next_cluster;
  uint32_t current_cluster;
} Cluster_t;

typedef struct ClusterChain {
  uint32_t start_cluster;
  uint32_t current_seq_cluster;
  Cluster_t cluster;
} ClusterChain_t;

typedef struct FAT_SectorCache {
  uint32_t sector_num;
  uint8_t usage_count;
  bool dirty : 1;
  bool busy : 1;
  uint8_t sector_data[512];
} FAT_SectorCache_t;

typedef struct FAT_Handle {
  uint8_t handle_type; // 0 = none, 1 = file, 2 = directory
  ClusterChain_t cluster_chain;
  uint32_t sector_offset;
  uint32_t size;
  union {
    struct {
      uint32_t current_cluster;
      uint32_t current_offset;
      uint32_t file_size;
    } file;
    struct {
      uint32_t current_cluster;
      uint32_t current_offset;
    } directory;
  };
} FAT_Handle_t;

typedef struct FAT_FileSystem FAT_FileSystem_t;

typedef union {
  struct{
    char filename[8];
    char ext[3];
    uint8_t attr;
    uint8_t nt_res;
    uint8_t crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t last_access_date;
    uint16_t high_word_of_cluster;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t low_word_of_cluster;
    uint32_t file_size;
  } __attribute__((packed));
  struct{
    uint8_t ldir_ord;
    uint8_t lfn1[5*2];
    uint8_t attr;
    uint8_t ldir_type;
    uint8_t ldir_chksum;
    uint8_t lfn2[6*2];
    uint16_t ldir_fstclust;
    uint8_t lfn3[2*2];
  } __attribute__((packed)) lfn;
} __attribute__((packed)) FAT_DirectoryEntry_t;


extern FileInfo_t dir_entry;


#define _(...)
#define FAT_FUNCTION_EXPORTS(modname, o) \
  FS_FUNCTION_INTERFACE(modname, o)\
  o(modname, init          , void              , FAT_FileSystem_t*, BlockDev *bd)                                 \
  o(modname, opendir       , file_descriptor_t , FAT_FileSystem_t*, const char*)                \
  o(modname, opendirat     , file_descriptor_t , FAT_FileSystem_t*, file_descriptor_t, const char*)                \
  o(modname, closedir      , void              , FAT_FileSystem_t*, file_descriptor_t)                \
  o(modname, readdir       , fstatus_t         , FAT_FileSystem_t*, file_descriptor_t, struct FileInfo*)                \


#define FAT_API_VER 1
#define FAT_MODULE_ID 0x0131

DECLARE_MODULE(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS);

struct FAT_FileSystem {
  union{
    fat_fns_t *fns;
    FileSystem_t fs;
  };
  const BlockDev *bd;
  FAT_SectorCache_t cache[1];
  uint32_t reserved_sectors;
  uint32_t num_FATs;
  uint32_t sectors_per_FAT;
  uint32_t sectors_per_cluster;
  uint32_t root_entries;
  uint32_t root_cluster;
  FAT_Handle_t handles[8];
};
