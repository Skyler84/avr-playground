#include "fat/fat.h"
#include "module/pic.h"
#include "module/imports.h"
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#define ATTR_READ_ONLY     0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04 
#define ATTR_VOLUME_ID     0x08
#define ATTR_DIRECTORY     0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_LONG_NAME      (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

MODULE_FN_PROTOS(fat, FAT_FUNCTION_EXPORTS)
static fstatus_t fat_readdir(FAT_FileSystem_t*, file_descriptor_t, struct FileInfo*);

#define FAT_MAX_HANDLES(fs) 8

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

static uint32_t umulhisi3(uint16_t a, uint16_t b)
{
  uint32_t out;
    asm volatile(
        "movw r26, %1\n"
        "movw r18, %2\n"
        "icall\n"
        "movw %A0, r22\n"
        "movw %C0, r24\n"
        : "=r" (out)
        : "r" (a), "r" (b), "z" (indirect_call(__umulhisi3))
        : "r26", "r27", "r18", "r19", "r22", "r23", "r24", "r25"
    );
    return out;
}
static uint32_t fat_cluster_sector_start(FAT_FileSystem_t *fs, uint32_t cluster) {
  if (cluster == 0) {
    return fs->reserved_sectors + indirect_call(__mulsi3)(fs->num_FATs, fs->sectors_per_FAT);
  } else {
    return indirect_call(fat_data_start_sector)(fs) + indirect_call(umulhisi3)((cluster - 2), fs->sectors_per_cluster);
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

static file_descriptor_t fat_allocate_handle(FAT_FileSystem_t *fs) {
  for (uint8_t i = 0; i < FAT_MAX_HANDLES(fs); i++) {
    if (fs->handles[i].handle_type == 0) {
      fs->handles[i].handle_type = 1;
      return i;
    }
  }
  return -1; // no free handle
}

static FAT_Handle_t *fat_get_handle(FAT_FileSystem_t *fs, file_descriptor_t fd) {
  if (fd < 0 || fd >= FAT_MAX_HANDLES(fs)) return NULL;
  return &fs->handles[fd];
}

static void fat_free_handle(FAT_FileSystem_t *fs, file_descriptor_t fd) {
  if (fd < 0 || fd >= FAT_MAX_HANDLES(fs)) return;
  fs->handles[fd].handle_type = 0;
  fs->handles[fd].cluster_chain.start_cluster = 0;
  fs->handles[fd].sector_offset = 0;
  fs->handles[fd].size = 0;
}

static void fat_duplicate_handle(FAT_FileSystem_t *fs, file_descriptor_t fd, file_descriptor_t newfd) {
  FAT_Handle_t *handle = fat_get_handle(fs, fd);
  FAT_Handle_t *newhandle = fat_get_handle(fs, newfd);
  if (handle == NULL) return;
  if (newhandle == NULL) return; // no free handle
  *newhandle = *handle; // copy the handle
}

static fstatus_t fat_navigate_directory(FAT_FileSystem_t *fs, file_descriptor_t fd, const char *dirname) 
{
  FAT_Handle_t *handle = fat_get_handle(fs, fd);
  while(*dirname != '\0') {
    if (handle->handle_type != 2) {
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
    fat_seek(&fs->fs, fd, 0, SEEK_SET);
    uint8_t i = 0;
    while ((ret = fat_readdir(fs, fd, &dir_entry)) == 0) {
      i++;
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
      if(diff) continue;
      fs->debug = cluster_num;
      // found the file
      
      fat_cluster_chain_init(fs, &handle->cluster_chain, cluster_num);
      handle->sector_offset = 0;
      if (dir_entry.type == 2) {
        handle->handle_type = 2; // directory
      } else {
        handle->handle_type = 1; // file
      }
      handle->size = dir_entry.size;
      i = 0;
      break;
    }
    if (ret != 0) {
      // reached end of directory.
      return -5+ret;
    }
    dirname = next;
  }
  return 0;
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
  for (uint8_t i = 0; i < FAT_MAX_HANDLES(fs); i++) {
    fat_free_handle(fs, i);
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
  
  // we don't support relative paths. 
  // relative paths are handled by the VFS layer
  // we only support absolute paths 
  if (filename[0] != '/') {
    return -1; // invalid path
  }
  return fat_openat(fs, -1, filename, mode);
}
file_descriptor_t fat_openat(FileSystem_t *_fs, file_descriptor_t dir, const char *filename, uint8_t mode) 
{ 
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  (void)fs;
  (void)dir;
  (void)filename;
  (void)mode;
  
  uint32_t dir_cluster;

  // first check if we have available handles
  file_descriptor_t i = fat_allocate_handle(fs);
  if (i == (file_descriptor_t)-1) {
    return -1; // no available handles
  }
  FAT_Handle_t *handle = &fs->handles[i];
  fstatus_t ret = 0;


  handle->handle_type = 2; // directory
  handle->size = -1;
  if (*filename == '/') {
    // absolute path
    dir_cluster = 0;
    filename++;
  } else {
    if (fs->handles[dir].handle_type != 2) {
      ret = -5; // not a directory
      goto err;
    }
    dir_cluster = fs->handles[dir].cluster_chain.start_cluster;
  }

  fs->handles[i].sector_offset = 0; // start at beginning of directory
  // get root dir clusterchain
  fat_cluster_chain_init(fs, &fs->handles[i].cluster_chain, dir_cluster);
  if ((ret = indirect_call(fat_navigate_directory)(fs, i, filename)) != 0) {
    ret -= 10;
    goto err;
  }
  if (fs->handles[i].handle_type == 2 && (mode & O_DIRECTORY) == 0) {
    ret = -3;
    goto err;
  }
  if (fs->handles[i].handle_type == 1 && (mode & O_DIRECTORY) != 0) {
    ret = -4;
    goto err;
  }
  return i;
err:
  fat_free_handle(fs, i);
  return ret; // error
}
void fat_close(FileSystem_t *_fs, file_descriptor_t fd) 
{
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  if (fs->handles[fd].handle_type == 0) {
    return; // already closed
  }
  fat_flush_sector(fs, fs->handles[fd].cluster_chain.cluster.sector_start);
  fat_free_handle(fs, fd);
}
fstatus_t fat_seek(FileSystem_t *_fs, file_descriptor_t fd, uint32_t offset, int whence) 
{ 
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  (void)fs;
  (void)fd;
  (void)offset;

  if (whence == SEEK_SET && offset == 0) {
    
    fat_cluster_chain_init(fs, &fs->handles[fd].cluster_chain, fs->handles[fd].cluster_chain.start_cluster);
    fs->handles[fd].sector_offset = 0;
    return 0;
  }

  return -1; 
}
fstatus_t fat_read(FileSystem_t *_fs, file_descriptor_t fd, char *_buf, uint16_t size) 
{ 
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  char *buf = _buf;
  FAT_SectorCache_t *cache;
  uint32_t offset;
  uint32_t remaining = -1;
  if ((fs->handles[fd].cluster_chain.cluster.current_cluster & 0x0ffffff8) == 0x0ffffff8) {
    return 0; // end of file
  }
  // if (fs->handles[fd].file.current_offset >= fs->handles[fd].size) {
  //   return 0; // end of file
  // }
  // remaining = fs->handles[fd].size - fs->handles[fd].file.current_offset;
  size = size > remaining ? remaining : size;
  do{
    cache = fat_cache_sector(fs, fs->handles[fd].cluster_chain.cluster.sector_start);
    offset = fs->handles[fd].sector_offset;
    for (; offset < 512 && size; size--, buf++, offset++, fs->handles[fd].file.current_offset++) {
      *buf = cache->sector_data[offset];
    }
    if (offset == 512) {
      fs->handles[fd].sector_offset = 0;
      fs->handles[fd].cluster_chain.cluster.sector_start++;
      fs->handles[fd].cluster_chain.cluster.sector_count--;
      if ( fs->handles[fd].cluster_chain.cluster.sector_count == 0) {
        fat_cluster_init(fs, &fs->handles[fd].cluster_chain.cluster, fs->handles[fd].cluster_chain.cluster.next_cluster);
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

fstatus_t fat_readdir(FAT_FileSystem_t *fs, file_descriptor_t fd, struct FileInfo *entry) 
{ 

  FAT_DirectoryEntry_t dir_entry;
  const char *si;
  char *di;
  int i = 0;
  while(i++<64) {
    fstatus_t ret = 32;
    ret = fat_read(&fs->fs, fd, (char*)&dir_entry, sizeof(FAT_DirectoryEntry_t));
    // ret = MODULE_CALL_THIS(fs, read, &fs->fs, fd, (char*)&dir_entry, sizeof(FAT_DirectoryEntry_t));
    if (ret < (long)sizeof(FAT_DirectoryEntry_t)) {  
      return -i; // error reading directory
    }
    if ((dir_entry.attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
      // long name entry
      if (dir_entry.lfn.ldir_ord & 0x40) {
        // first entry in LFN
        entry->name[0] = '\0';
      }
      memmove(&entry->name[13], &entry->name[0], sizeof(entry->name) - 13);
      uint8_t n = 0;
      for (uint8_t i = 0; i < 5; i++) {
        entry->name[n++] = dir_entry.lfn.lfn1[i*2];
      }
      for (uint8_t i = 0; i < 6; i++) {
        entry->name[n++] = dir_entry.lfn.lfn2[i*2];
      }
      for (uint8_t i = 0; i < 2; i++) {
        entry->name[n++] = dir_entry.lfn.lfn3[i*2];
      }
      continue;
      static_assert(offsetof(FAT_DirectoryEntry_t, lfn.lfn1) == 1, "LFN1 offset is not correct");
      static_assert(offsetof(FAT_DirectoryEntry_t, lfn.lfn2) == 14, "LFN2 offset is not correct");
      static_assert(offsetof(FAT_DirectoryEntry_t, lfn.lfn3) == 28, "LFN3 offset is not correct");
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
    si = dir_entry.filename;
    di = entry->altname;
    if (dir_entry.attr & 0x10) {
      entry->type = FI_TYPE_DIR; // directory
      for (int i = 0; i < 8; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = tolower(*si++);
      }
    } else {
      entry->type = FI_TYPE_FILE; // file
      for (int i = 0; i < 8; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = tolower(*si++);
      }
      *di++ = '.';
      si = dir_entry.ext;
      for (int i = 0; i < 3; i++) {
        if (*si == ' ') {
          break;
        }
        *di++ = tolower(*si++);
      }
    }
    *di = '\0';
    // check if altname matches name
    // if it doesn't then copy alt name into name.
    for (uint8_t i = 0; i < 13; i++) {
      if (entry->altname[i] == 0 && entry->name[i] == 0) {
        break;
      }
      if (entry->altname[i] == '~') {
        break;
      }
      if (indirect_call(toupper)(entry->altname[i]) == indirect_call(toupper)(entry->name[i])) {
      // if (toupper(entry->altname[i]) == toupper(entry->name[i])) {
        continue;
      }
      strcpy(entry->name, entry->altname);
      break;
    }
    entry->size = dir_entry.file_size;
    if (dir_entry.attr & 0x10) {
      entry->size = -1;
    }
    entry->inode = (uint32_t)dir_entry.high_word_of_cluster << 16 | dir_entry.low_word_of_cluster;
    return 0;
  }

  return -9; // not implemented
}

fstatus_t fat_getdirents(FileSystem_t *_fs, file_descriptor_t fd, struct FileInfo *entry, uint16_t count) 
{ 
  FAT_FileSystem_t *fs = (FAT_FileSystem_t*)_fs;
  (void)fs;
  (void)fd;
  (void)entry;
  (void)count;
  if (fs->handles[fd].handle_type != 2) {
    return -1; // not a directory
  }
  
  if (entry == NULL) {
    fat_seek(_fs, fd, 0, SEEK_SET);
    return 0;
  }

  FAT_DirectoryEntry_t dir_entry;
  for(int i = 0; i < 32; i++) {
    ((uint8_t*)&dir_entry)[i] = 0;
  }
  uint16_t i = 0;
  uint8_t failsafe = 32;
  while(i < count && failsafe--) {
    fstatus_t ret = 32;
    ret = indirect_call(fat_readdir)(fs, fd, entry);
    if (ret < 0) {
      return i;
    }
    i++;
    entry++;
    continue;
  }


  return i; 
}

REGISTER_MODULE(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, FAT_API_VER);