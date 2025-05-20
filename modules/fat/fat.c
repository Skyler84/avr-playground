#include "fat/fat.h"
#include "module/pic.h"

MODULE_FN_PROTOS(fat, FAT_FUNCTION_EXPORTS)

#define FAT_MAX_HANDLES(fs) 4

int __mulsi3(int a, int b)
{
  return a*b;
}


static FAT_SectorCache_t* fat_cache_sector(FAT_FileSystem_t *fs, uint32_t sector) {
  for (uint8_t i = 0; i < 1; i++) {
    if (fs->cache[i].sector_num == sector) {
      if (fs->cache[i].usage_count < 255)
        fs->cache[i].usage_count++;
      return &fs->cache[i];
    }
  }
  // find someone to evict
  FAT_SectorCache_t *cache = &fs->cache[0];
  for (uint8_t i = 1; i < 1; i++) {
    if (fs->cache[i].usage_count < cache->usage_count) {
      cache = &fs->cache[i];
    }
  }
  if (cache->dirty) {
    fs->bd->fns->write_sector(fs->bd, cache->sector_num + fs->bd->sector_start, cache->sector_data);
  }
  for (uint8_t i = 0; i < 1; i++) {
    fs->cache[i].usage_count -= cache->usage_count;
  }
  cache->sector_num = sector;
  cache->usage_count = 1;
  cache->dirty = false;
  fs->bd->fns->read_sector(fs->bd->fn_ctx, sector + fs->bd->sector_start, cache->sector_data);
  return cache;
}

// static void fat_write_sector(FAT_FileSystem_t *fs, uint32_t sector) {
// }

static void fat_flush_sector(FAT_FileSystem_t *fs, uint32_t sector) {
  
  FAT_SectorCache_t *cache = NULL;
  for (uint8_t i = 0; i < 1; i++) {
    if (fs->cache[i].sector_num == sector) {
      cache = &fs->cache[i];
      break;
    }
  }
  if (cache == NULL) return;
  if (cache->dirty) {
    fs->bd->fns->write_sector(fs->bd->fn_ctx, sector, cache->sector_data);
    cache->dirty = false;
  }
}

// static uint32_t fat_root_start_sector(FAT_FileSystem_t *fs) {
//   uint32_t reserved_sectors = fs->reserved_sectors;
//   uint32_t num_FATs = fs->num_FATs;
//   uint32_t sectors_per_FAT = fs->sectors_per_FAT;
//   return reserved_sectors + (num_FATs * sectors_per_FAT);
// }

static uint32_t fat_data_start_sector(FAT_FileSystem_t *fs) {
  uint32_t reserved_sectors = fs->reserved_sectors;
  uint32_t num_FATs = fs->num_FATs;
  uint32_t sectors_per_FAT = fs->sectors_per_FAT;
  uint32_t root_dir_start_sector = reserved_sectors + indirect_call(__mulsi3)(num_FATs, sectors_per_FAT);
  uint32_t root_dir_sectors = 0; // (fs->sectors_per_cluster * 32 + fs->sectors_per_cluster - 1) / fs->sectors_per_cluster;
  return root_dir_start_sector + root_dir_sectors;
}

static uint32_t fat_root_start_sector(FAT_FileSystem_t *fs) {
  if (fs->root_cluster == 0) {
    return fs->reserved_sectors + indirect_call(__mulsi3)(fs->num_FATs, fs->sectors_per_FAT);
  } else {
    return fat_data_start_sector(fs) + indirect_call(__mulsi3)((fs->root_cluster - 2), fs->sectors_per_cluster);
  }
}

static uint32_t fat_cluster_sector_start(FAT_FileSystem_t *fs, uint32_t cluster) {
  if (cluster == 0) {
    return fs->reserved_sectors + indirect_call(__mulsi3)(fs->num_FATs, fs->sectors_per_FAT);
  } else {
    return indirect_call(fat_data_start_sector)(fs) + indirect_call(__mulsi3)((cluster - 2), fs->sectors_per_cluster);
  }
}

static __attribute__((noinline)) uint32_t fat_cluster_chain_next(FAT_FileSystem_t *fs, uint32_t cluster) {
  // cluster -= 2; // first cluster is 2
  uint32_t fat_sector = fs->reserved_sectors + (cluster / 128);
  FAT_SectorCache_t *cache = fat_cache_sector(fs, fat_sector);
  uint32_t offset = (cluster % 128) * 4;
  return *((uint32_t*)&cache->sector_data[offset]);
}

static void fat_cluster_init(FAT_FileSystem_t *fs, Cluster_t *cluster, uint32_t cluster_no) {
  if (cluster_no == 0) {
    cluster_no = fs->root_cluster;
    cluster->sector_start = fat_root_start_sector(fs);
  } else {
    cluster->sector_start = fat_cluster_sector_start(fs, cluster_no);
  }
  cluster->current_cluster = cluster_no;
  if (cluster_no == 0) {
    cluster->next_cluster = 0x0ffffff8; // end of chain
    // cluster->sector_count = indirect_call(__mulsi3)(fs->root_entries, 16 + fs->sectors_per_cluster-1)/ fs->sectors_per_cluster;
    cluster->sector_count = indirect_call(__udivmodsi4)(indirect_call(__mulsi3)(fs->root_entries, 16 + fs->sectors_per_cluster-1), fs->sectors_per_cluster, 0);
  } else {
    cluster->sector_count = fs->sectors_per_cluster;
    cluster->next_cluster = indirect_call(fat_cluster_chain_next)(fs, cluster_no);
  }
}

static void fat_cluster_chain_init(FAT_FileSystem_t *fs, ClusterChain_t *chain, uint32_t start_cluster) {
  chain->start_cluster = start_cluster;
  chain->current_seq_cluster = 0;
  indirect_call(fat_cluster_init)(fs, &chain->cluster, start_cluster);
}

static file_descriptor_t fat_get_handle(FAT_FileSystem_t *fs) {
  for (uint8_t i = 0; i < FAT_MAX_HANDLES(fs); i++) {
    if (fs->handles[i].handle_type == 0) {
      fs->handles[i].handle_type = 1;
      return i;
    }
  }
  return -1; // no free handle
}

static fstatus_t fat_navigate_directory(FAT_FileSystem_t *fs, file_descriptor_t fd, char *dirname) 
{
  while(*dirname != '\0') {
    if (fs->handles[fd].handle_type != 2) {
      return -1; // not a directory
    }
    // find the next / in the path
    char *next = dirname;
    while (*next != '\0' && *next != '/') {
      next++;
    }
    // if we have a /, then we need to null terminate the string
    char lastchar = *next;
    if (*next == '/') {
      // *next = '\0';
      next++;
    }
    // now we need to find the file in the directory
    FileInfo_t dir_entry;
    fstatus_t ret = 0;
    indirect_call(fat_readdir)(fs, fd, NULL);
    while ((ret = indirect_call(fat_readdir)(fs, fd, &dir_entry)) == 0) {
      uint32_t cluster_num = dir_entry.inode;
      if (dir_entry.type != 2 && lastchar == '/') {
        // not a directory
        continue;
      }
      char diff = 0;
      const char *s1 = dirname, *s2 = dir_entry.name;
      #define PATHSTR_END(x) (*x == '\0' || *x == '/')
      while (!PATHSTR_END(s1) || *s2) {
        diff |= *s1 ^ *s2;
        if (!PATHSTR_END(s1)) s1++;
        if (*s2) s2++;
      }
      if (diff == 0) {
        // found the file
        fat_cluster_chain_init(fs, &fs->handles[fd].cluster_chain, cluster_num);
        fs->handles[fd].sector_offset = 0;
        if (dir_entry.type == 2) {
          fs->handles[fd].handle_type = 2; // directory
        } else {
          fs->handles[fd].handle_type = 1; // file
        }
        break;
      }
    }
    if (ret != 0) {
      // reached end of directory.
      goto err;
    }
    dirname = next;
  }
  return 0;
err:
  return -1; // error
}

void fat_init(FAT_FileSystem_t *fs, BlockDev *bd) 
{
  fs->bd = bd;
  fs->num_FATs = 0;
  fs->sectors_per_FAT = 0;
  fs->sectors_per_cluster = 0;
  for (uint8_t i = 0; i < 1; i++) {
    fs->cache[i].sector_num = -1;
    fs->cache[i].usage_count = 0;
    fs->cache[i].dirty = false;
  }
  for (uint8_t i = 0; i < 4; i++) {
    fs->handles[i].handle_type = 0;
  }
}

fstatus_t fat_mount(FileSystem_t *_fs, bool readonly, bool mkfs) 
{
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  (void)readonly;
  (void)mkfs;
  
  if (fs->bd == NULL) {
    return -3; // not initialized
  }
  FAT_SectorCache_t *cache = fat_cache_sector(fs, 0);
  fstatus_t ret = 0;
  cache->busy = 1;
  if (cache->sector_data[510] != 0x55 || cache->sector_data[511] != 0xAA) {
    ret = -2; // not a valid FAT filesystem
    goto err;
  }
  BPB2_t *bpb2 = (BPB2_t*)(&cache->sector_data[0x0b]);
  BPB3_31_t *bpb3 = (BPB3_31_t*)(&cache->sector_data[0x0b]);
  FAT32EBPB_t *fat32ebpb = (FAT32EBPB_t*)(&cache->sector_data[0x0b]);
  
  uint8_t sectors_per_cluster = bpb2->sectors_per_cluster;
  uint32_t total_sectors = bpb2->total_sectors_16;
  // uint32_t bytes_per_sector = bpb2->bytes_per_sector;
  if (total_sectors == 0) {
    total_sectors = bpb3->total_sectors_32;
  }
  uint32_t reserved_sectors = bpb2->reserved_sectors;
  uint8_t num_fats = bpb2->num_fats;
  uint32_t fat_sectors = bpb2->sectors_per_fat;
  if (fat_sectors == 0)
    fat_sectors = fat32ebpb->sectors_per_fat;
  // uint32_t root_dir_start_sector = reserved_sectors + fat_sectors;
  // uint32_t root_dir_sectors = (bpb2->root_entries * 32 + bytes_per_sector - 1) / bytes_per_sector;
  // uint32_t data_start_sector = root_dir_start_sector + root_dir_sectors;

  fs->sectors_per_cluster = sectors_per_cluster;
  fs->reserved_sectors = reserved_sectors;
  fs->num_FATs = num_fats;
  fs->sectors_per_FAT = fat32ebpb->sectors_per_fat;
  fs->root_cluster = fat32ebpb->root_cluster;
err:
  cache->busy = 0;
  return ret;
}
void fat_umount(FileSystem_t *fs) 
{
  (void)fs;
}
fstatus_t fat_stat(FileSystem_t *fs, const char *filename, struct FileInfo *st) 
{ 
  (void)fs;
  (void)filename;
  (void)st;

  return 0; 
}
file_descriptor_t fat_open(FileSystem_t *fs, const char *filename, uint8_t mode) 
{ 
  (void)fs;
  (void)filename;
  (void)mode;

  return 0; 
}
file_descriptor_t fat_openat(FileSystem_t *fs, file_descriptor_t dir, const char *filename, uint8_t mode) 
{ 
  (void)fs;
  (void)dir;
  (void)filename;
  (void)mode;
  
  return 0; 
}
void fat_close(FileSystem_t *_fs, file_descriptor_t fd) 
{
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  if (fs->handles[fd].handle_type != 1) {
    return; // already closed
  }
  fs->handles[fd].handle_type = 0;
  fs->handles[fd].sector_offset = 0;
  fs->handles[fd].size = 0;
  fat_flush_sector(fs, fs->handles[fd].cluster_chain.cluster.sector_start);
}
fstatus_t fat_seek(FileSystem_t *fs, file_descriptor_t fd, uint32_t offset) 
{ 
  (void)fs;
  (void)fd;
  (void)offset;

  return 0; 
}
fstatus_t fat_read(FileSystem_t *_fs, file_descriptor_t fd, char *_buf, uint16_t size) 
{ 
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  if (fs->handles[fd].handle_type != 1) {
    return -1; // not a file
  }
  char *buf = _buf;
  FAT_SectorCache_t *cache;
  uint32_t offset;
  do{
    cache = fat_cache_sector(fs, fs->handles[fd].cluster_chain.cluster.sector_start);
    offset = fs->handles[fd].sector_offset;
    for (; offset < 512 && size; size--, buf++, offset++) {
      *buf = cache->sector_data[offset];
    }
    if (offset == 512) {
      fs->handles[fd].sector_offset = 0;
      // offset = 0;
      fs->handles[fd].cluster_chain.cluster.sector_start++;
      fs->handles[fd].cluster_chain.cluster.sector_count--;
      if ( fs->handles[fd].cluster_chain.cluster.sector_count == 0) {
        indirect_call(fat_cluster_init)(fs, &fs->handles[fd].cluster_chain.cluster, fs->handles[fd].cluster_chain.cluster.next_cluster);
        fs->handles[fd].cluster_chain.current_seq_cluster++;
      }
    } else {
      fs->handles[fd].sector_offset = offset;
    }
  } while (size && (fs->handles[fd].cluster_chain.cluster.current_cluster & 0x0ffffff8ul) != 0x0ffffff8ul);
  return buf - _buf; 
}
fstatus_t fat_write(FileSystem_t *fs, file_descriptor_t fd, const char *buf, uint16_t size) 
{ 
  (void)fs;
  (void)fd;
  (void)buf;
  (void)size;

  return 0; 
}
fstatus_t fat_unlink(FileSystem_t *fs, const char *filename) 
{ 
  (void)fs;
  (void)filename;

  return 0; 
}
fstatus_t fat_rename(FileSystem_t *fs, const char *oldname, const char *newname) 
{ 
  (void)fs;
  (void)oldname;
  (void)newname;

  return 0; 
}
void fat_mkdir(FileSystem_t *fs, const char *dirname) 
{
  (void)fs;
  (void)dirname;
}
void fat_rmdir(FileSystem_t *fs, const char *dirname) 
{
  (void)fs;
  (void)dirname;
}

file_descriptor_t fat_opendir(FAT_FileSystem_t *fs, const char *dirname) 
{ 
  // we don't support relative paths. 
  // relative paths are handled by the VFS layer
  // we only support absolute paths 
  if (dirname[0] != '/') {
    return -1; // invalid path
  }
  return fat_opendirat(fs, -1, dirname);
}
file_descriptor_t fat_opendirat(FAT_FileSystem_t *fs, file_descriptor_t dir, const char *dirname) 
{
  uint32_t dir_cluster;
  if (*dirname == '/') {
    // absolute path
    dir_cluster = 0;
    dirname++;
  } else {
    if (fs->handles[dir].handle_type != 2) {
      return -1; // not a directory
    }
    dir_cluster = fs->handles[dir].cluster_chain.start_cluster;
  }

  
  // first check if we have available handles
  file_descriptor_t i = fat_get_handle(fs);
  if (i == (file_descriptor_t)-1) {
    return i; // no available handles
  }
  
  fs->handles[i].handle_type = 2; // directory
  fs->handles[i].sector_offset = 0; // start at beginning of directory
  fs->handles[i].size = 0; // size is unknown
  // get root dir clusterchain
  fat_cluster_chain_init(fs, &fs->handles[i].cluster_chain, dir_cluster);
  if (indirect_call(fat_navigate_directory)(fs, i, (char*)dirname) != 0) {
    goto err;
  }
  if (fs->handles[i].handle_type != 2) {
    goto err;
  }
  return i;
err:
  fs->handles[i].handle_type = 0; // invalid handle
  return -2; // error
}
void fat_closedir(FAT_FileSystem_t *fs, file_descriptor_t fd) 
{
  if (fs->handles[fd].handle_type != 2) {
    return; // not a directory
  }
  fs->handles[fd].handle_type = 0; // invalid handle
  // fs->handles[fd].sector_offset = 0;
  // fs->handles[fd].size = 0;
  // fs->handles[fd].cluster_chain.start_cluster = 0;
  // fs->handles[fd].cluster_chain.current_seq_cluster = 0;
}
fstatus_t fat_readdir(FAT_FileSystem_t *fs, file_descriptor_t fd, struct FileInfo *entry) 
{ 
  if (fs->handles[fd].handle_type != 2) {
    return -1; // not a directory
  }

  if (entry == NULL) {
    // reset directory to beginning
    fat_cluster_chain_init(fs, &fs->handles[fd].cluster_chain, fs->handles[fd].cluster_chain.start_cluster);
    fs->handles[fd].sector_offset = 0;
    return 0;
  }

  FAT_DirectoryEntry_t dir_entry;
  for(int i = 0; i < 32; i++) {
    ((uint8_t*)&dir_entry)[i] = 0;
  }
  const char *si;
  char *di;
  int i = 0;
  while(i++<32) {
    fstatus_t ret = 32;
    fs->handles[fd].handle_type = 1;
    ret = indirect_call(fat_read)(&fs->fs, fd, (char*)&dir_entry, sizeof(FAT_DirectoryEntry_t));
    fs->handles[fd].handle_type = 2;
    if (ret < (long)sizeof(FAT_DirectoryEntry_t)) {  
      return -i; // error reading directory
    }
    if ((uint8_t)dir_entry.filename[0] == (uint8_t)0x00) {
      continue;
    }
    if ((uint8_t)dir_entry.filename[0] == (uint8_t)0xE5) {
      continue; // deleted file
    }
    if (dir_entry.attr & ~0x30) {
      // return -4;
      continue; // not file or dir
    }
    if (dir_entry.attr & 0x10) {
      entry->type = 2; // directory
      si = dir_entry.filename;
      di = entry->name;
      for (int i = 0; i < 8; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = *si++;
      }
    } else {
      entry->type = 1; // file
      si = dir_entry.filename;
      di = entry->name;
      for (int i = 0; i < 8; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = *si++;
      }
      *di++ = '.';
      si = dir_entry.ext;
      for (int i = 0; i < 3; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = *si++;
      }
    }
    *di = '\0';
    entry->size = dir_entry.file_size;
    entry->inode = (uint32_t)dir_entry.high_word_of_cluster << 16 | dir_entry.low_word_of_cluster;
    return 0;
  }


  return -9; // not implemented
}

REGISTER_MODULE(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, FAT_API_VER);