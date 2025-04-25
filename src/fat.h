/* 
 * File:   fat_.h
 * Author: krik
 *
 * Created on November 11, 2013, 8:55 PM
 */

#ifndef FAT__H
#define	FAT__H

#include "types.h"

#define FAT_TYPE_16 1
#define FAT_TYPE_32 2
#define FAT_END_VAL 0x0fffffff

typedef struct {
    u8 name[212];
    u32 hdr_entry;
    u32 hdr_entry_8;
    u32 data_clsut;
    u32 size;
    u8 is_dir;
} FatFullRecord;

typedef struct {
    u32 cluster;
    u32 sector;
    u32 size;
    u16 sec_available;
    u8 wr_slen;
    u8 in_cluster;

} File;

u8 fat_init();
u8 fat_load_dir(u32 entry);
u8 fat_get_full_record(u32 hdr_entry, FatFullRecord *rec);
u8 fat_open_file(FatFullRecord *rec, u16 wr_slen);
u8 fat_skip_sectors(u16 slen);
u8 fat_read(u8 *dst, u16 slen, u8 memory);
void fat_make_path_name(u8 *dst, u8 *dir, u8 *filename);
void fat_make_sync_name(u8 * dir, u8 *filename, u8 *ext, u8 *dst);
u8 fat_open_file_by_name(u8 *name, FatFullRecord *rec, u16 wr_slen);
u8 fat_write_file(u8 *src, u16 slen);
u8 fat_make_dir(FatFullRecord *rec, u8 *name);
u32 fat_cluster_to_sector(u32 cluster);
void fat_bytes_to_int_le(u8 *src, u32 *dst);
void fat_int_to_bytes_le(u32 src, u8 *dst);

extern u32 fat_sub_dir_clust;
extern u16 fat_dir_size;
extern u32 *fat_dir;
extern File file;



//****************************************************************************** fat wr
u8 fat_seek_free_cluster(u32 *base_clust, u16 clen);

#endif	/* FAT__H */

