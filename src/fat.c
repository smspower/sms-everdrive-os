

#include "fat.h"
#include "mmc.h"
#include "vdp.h"
#include "joy.h"


void fatGetLongNamePart(u8 *name, u8 *src);
void fatSortDir();

u32 fat_pbr_ptr;
u8 fat_sectors_per_cluster;
u8 fat_reserved_sectors;
u16 fat_sectors_per_fat;
u32 fat_root_ptr;
u32 fat_table_ptr;
u32 fat_data_start_ptr;
u16 fat_table_buff[256];
u8 fat_current_table_buff;
u32 fat_cluster_size;


FatFile file;
FatDir dir;

#define ENTY_TO_ADDR(entry) ((u32)(entry - 2) * fat_cluster_size + fat_data_start_ptr)

u8 fatInit() {

    u8 resp;
    u8 yy = 5;
    u8 mbr = 0;
    u8 i;

    fat_pbr_ptr = 0;
    resp = mmcInit();
    if (resp)return 1;


    resp = mmcRdBlock(0, mmc_buff);
    if (resp)return 2;

    //resp = mmcRdBlock(0, mmc_buff);
    for (i = 0; i < 5; i++) {
        if (mmc_buff[i + 0x36] != ((u8 *) "FAT16")[i]) {
            mbr = 1;
        }
    }
    if (mbr == 1) {
        fat_pbr_ptr = (u32) mmc_buff[0x1c6] | (u32) mmc_buff[0x1c7] << 8 | (u32) mmc_buff[0x1c8] << 16 | (u32) mmc_buff[0x1c9] << 24;
        fat_pbr_ptr *= 512;
        resp = mmcRdBlock(0, mmc_buff);
        if (resp)return 3;
    }


    fat_sectors_per_cluster = mmc_buff[0x0d];
    fat_reserved_sectors = mmc_buff[0x0e];
    fat_sectors_per_fat = mmc_buff[0x16] | (mmc_buff[0x17] << 8);
    fat_root_ptr = (u32) (fat_sectors_per_fat << 1) + fat_reserved_sectors << 9;
    //fat_root_ptr += pbr_ptr;
    fat_table_ptr = (u32) fat_reserved_sectors << 9;
    //fat_table_ptr += pbr_ptr;
    fat_data_start_ptr = (u32) fat_root_ptr + 16384;
    //fat_data_start_ptr += pbr_ptr;
    fat_cluster_size = (u32) fat_sectors_per_cluster * 512;
    /*
        vdpDrawStr(&mmc_buff[3], 2, yy++, PAL1);
        vdpDrawNum("spc: ", fat_sectors_per_cluster, 2, yy++, PAL1);
        vdpDrawNum("res: ", fat_reserved_sectors, 2, yy++, PAL1);
        vdpDrawNum("spf: ", fat_sectors_per_fat, 2, yy++, PAL1);
        vdpDrawNum("roo: ", fat_root_ptr, 2, yy++, PAL1);
        vdpDrawNum("fpt: ", fat_table_ptr, 2, yy++, PAL1);
        vdpDrawNum("dts: ", fat_data_start_ptr, 2, yy++, PAL1);
        vdpDrawNum("cls: ", fat_cluster_size, 2, yy++, PAL1);
        vdp_repaint();
        joy_wait_read();

     */
    return 0;
}

#define CN_LOAD_FLAGS 1
#define CN_LOAD_PART1 2

u8 fatOpenDir(u16 entry) {


    u8 is_root = entry == 0 ? 1 : 0;
    u32 addr = is_root ? fat_root_ptr : ENTY_TO_ADDR(entry);
    u8 i;
    u8 u;
    u8 *sector_ptr;
    u8 continue_operation = 0;
    u8 *name_ptr;
    u8 stop_loop = 0;
    u8 sector_ctr = 0;

    ///u16 sector_ctr;

    for (dir.size = 0; dir.size < FAT_DIR_SIZE;) {

        if (stop_loop)break;
        if (sector_ctr == 32 && is_root)break;
        if (sector_ctr == 128)break;
        sector_ctr++;

        if (mmcRdBlock(addr, mmc_buff) != 0)return 1;
        sector_ptr = (u8 *) mmc_buff;


        for (u = 0; u < 16; u++) {


            if (*sector_ptr == 0 || dir.size == FAT_DIR_SIZE) {
                stop_loop = 1;
                break;
            }

            if (*sector_ptr == 0xe5 || *sector_ptr == 0x2e) {
                sector_ptr += 32;
                continue;
            }

            if (continue_operation == CN_LOAD_FLAGS) {
                dir.records[dir.size].flags = sector_ptr[0x0b] & (0x20 | 0x10);
                dir.records[dir.size].entry = *((u16 *) & sector_ptr[0x1a]);
                dir.records[dir.size].size = *((u32 *) & sector_ptr[0x1c]);
                dir.records[dir.size].rec_addr = (u32) (u << 5) + addr;
                dir.size++;
                sector_ptr += 32;
                continue_operation = 0;
                continue;
            }


            if ((sector_ptr[0x0b] == 0x0f & (*sector_ptr & 0xf) == 1) || continue_operation == CN_LOAD_PART1) {
                fatGetLongNamePart(dir.records[dir.size].name, sector_ptr);
                continue_operation = CN_LOAD_FLAGS;
                sector_ptr += 32;
                continue;
            }

            if ((sector_ptr[0x0b] == 0x0f & (*sector_ptr & 0xf) == 2)) {
                fatGetLongNamePart(&dir.records[dir.size].name[13], sector_ptr);
                if (gg_mode) {
                    dir.records[dir.size].name[20] = 0;
                } else {
                    dir.records[dir.size].name[26] = 0;
                }

                continue_operation = CN_LOAD_PART1;
                sector_ptr += 32;
                continue;
            }




            if (sector_ptr[0x0b] == 0x10 || sector_ptr[0x0b] == 0x20) {

                name_ptr = (u8 *) dir.records[dir.size].name;
                for (i = 0; i < 12; i++)*name_ptr++ = sector_ptr[i];
                *name_ptr = 0;
                dir.records[dir.size].flags = sector_ptr[0x0b] & (0x20 | 0x10);
                dir.records[dir.size].entry = *((u16 *) & sector_ptr[0x1a]);
                dir.records[dir.size].size = *((u32 *) & sector_ptr[0x1c]);
                dir.records[dir.size].rec_addr = (u32) (u << 5) + addr;
                dir.size++;
                sector_ptr += 32;
                continue;
            }

            sector_ptr += 32;
        }

        addr += 512;
    }

    dir.entry = entry;

    fatSortDir();


    return 0;
}

void fatGetLongNamePart(u8 *name, u8 *src) {

    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *src++;
    *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *src++;
    *src++;
    *name++ = *src++;
    *src++;
    *name++ = *src++;

}

void fatSortDir() {

    u8 dir_num = 0;
    u8 i;
    u8 buff;
    u16 name0;
    u16 name1;
    u8 *n_ptr;

    for (i = 0; i < dir.size; i++) {
        dir.order[i] = i;
    }

    
    for (i = 1; i < dir.size;) {

        if (dir.records[dir.order[i]].flags == FAT_DIR && dir.records[dir.order[i - 1]].flags == FAT_FIL) {
            buff = dir.order[i];
            dir.order[i] = dir.order[i - 1];
            dir.order[i - 1] = buff;
            if (i > 1)i--;
            continue;
        }
        i++;
    }



    for (i = 0; i < dir.size; i++) {
        if (dir.records[dir.order[i]].flags != FAT_DIR)break;
    }
    dir_num = i;

    for (i = 1; i < dir_num;) {

        n_ptr = (u8 *) dir.records[dir.order[i - 1]].name;
        name0 = *n_ptr++ << 8;
        name0 |= *n_ptr++;
        name0 |= 0x02020;

        n_ptr = (u8 *) dir.records[dir.order[i]].name;
        name1 = *n_ptr++ << 8;
        name1 |= *n_ptr++;
        name1 |= 0x02020;

        if (name1 < name0) {
            buff = dir.order[i];
            dir.order[i] = dir.order[i - 1];
            dir.order[i - 1] = buff;
            if (i > 1)i--;
        } else {
            i++;
        }
    }

    for (i = dir_num + 1; i < dir.size;) {

        n_ptr = (u8 *) dir.records[dir.order[i - 1]].name;
        name0 = *n_ptr++ << 8;
        name0 |= *n_ptr++;
        name0 |= 0x02020;

        n_ptr = (u8 *) dir.records[dir.order[i]].name;
        name1 = *n_ptr++ << 8;
        name1 |= *n_ptr++;
        name1 |= 0x02020;

        if (name1 < name0) {
            buff = dir.order[i];
            dir.order[i] = dir.order[i - 1];
            dir.order[i - 1] = buff;
            if (i > (dir_num + 1) && i > 1)i--;
        } else {
            i++;
        }
    }


}

void fatOpenFile(FatRecord *rec) {

    file.record = rec;
    file.pos = 0;
    file.cluster = rec->entry;
    file.sector = 0;
    file.addr_buff = (u32) (file.cluster - 2) * fat_cluster_size + fat_data_start_ptr;

}

u8 fatReadNextBlock() {

    u8 resp;
    resp = mmcRdBlock(file.addr_buff, mmc_buff);
    if (resp != 0)return resp;
    file.addr_buff += 512;

    return 0;
}

u8 fatWriteNextBlock(u8 *data) {

    u8 resp;
    //resp = mmcRdBlock(file.addr_buff, mmc_buff);
    resp = mmcWrBlock(file.addr_buff, data);
    if (resp != 0)return resp;
    file.addr_buff += 512;

    return 0;
}
