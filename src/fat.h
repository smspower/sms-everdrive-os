/* 
 * File:   fat.h
 * Author: KRIK
 *
 * Created on 23 Декабрь 2010 г., 10:20
 */

#ifndef _FAT_H
#define	_FAT_H

#include "types.h"

#define FAT_DIR_SIZE 90
#define FAT_DIR 0x10
#define FAT_FIL 0x20

typedef struct {
    u8 name[27];
    u8 flags;
    u16 entry;
    u32 size;
    u32 rec_addr;
} FatRecord;

typedef struct {
    FatRecord records[FAT_DIR_SIZE];
    u16 entry;
    u16 size;
    u8 order[FAT_DIR_SIZE];
} FatDir;

typedef struct {
    FatRecord *record;
    u8 sectror_buff[512];
    u16 cluster;
    u32 pos;
    u32 addr_buff;
    u8 sector;
} FatFile;

u8 fatInit();
u8 fatOpenDir(u16 entry);
void fatOpenFile(FatRecord *rec);
u8 fatReadNextBlock();
u8 fatWriteNextBlock(u8 *data);

extern FatDir dir;
extern FatFile file;
extern u32 fat_pbr_ptr;

#endif	/* _FAT_H */

