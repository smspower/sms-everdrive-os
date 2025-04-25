

#include "fat.h"
#include "disk.h"
#include "str.h"
#include "os.h"
#include "error.h"
#include "sys.h"






#define MOTO_CPU


static const u8 lfn_char_struct[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};



u32 fat_clust_num;
u32 fat_entry;
u32 fat_pbr_entry;
u32 fat_dat_sector;
u32 fat_tab_sector;
u32 fat_sectors_per_fat;
u32 fat_root_entry;
u32 fat_data_entry;
u16 fat_reserved_sectors;
u8 fat_cluster_size;
u8 fat_type;
u8 fat_cluster_size_pow;


u32 fat_first_free_clust;
u32 fat_sub_dir_clust;
u16 fat_dir_size;
u8 fat_dir_is_root;

u8 *fat_dat;
u8 *fat_tab;

u32 *fat_dir;
File file;

u8 fat_cache_read_dat(u32 sector);
u8 fat_cache_read_tab(u32 sector);
u8 fat_get_type();

u16 fat_bytes_to_short(u8 *src);
u8 fat_get_next_cluster(u32 *cluster);

u32 fat_sector_to_cluster(u32 sector);
u8 fat_in_cluster_addr(u32 sector);
u8 fat_seek_record(u8 *name, FatFullRecord *rec, u32 *file_root, u8 dir);



u8 fat_make_record(u8 *name, u32 dir_entry, u16 slen, u8 is_dir);
u8 fat_resize_file(FatFullRecord *rec, u16 slen);
u8 fat_cache_save_tab();
u8 fat_catch_clusters(u32 base, u16 clen);
u8 fat_clean_cluster(u32 cluster, u16 clen);
u8 fat_join(u32 last_clust, u32 next_clust);
u8 fat_expand(u32 last_clust, u16 csize, u8 clean);
u8 fat_seek_rec_end(u32 start_clust, u32 *end_clust);
u8 fat_get_table_record(u32 cluster, u32 *val);

u8 fat_init() {

    u8 resp;
    fat_dat = (u8 *) osMallocRam(512);
    fat_tab = (u8 *) osMallocRam(512);
    fat_dir = (u32 *) osMallocRam(OS_MAX_DIR_SIZE * 4);

    fat_dat_sector = ~0;
    fat_tab_sector = ~0;

    fat_pbr_entry = 0;
    resp = fat_cache_read_dat(fat_pbr_entry);
    if (resp)return resp;
    fat_type = fat_get_type();

    if (fat_type == 0) {
        fat_bytes_to_int_le(&fat_dat[0x1c6], &fat_pbr_entry);
        resp = fat_cache_read_dat(fat_pbr_entry);
        if (resp)return resp;
        fat_type = fat_get_type();
        if (fat_type == 0)return FAT_ERR_UNK_TYPE;
    }

    if (fat_type == FAT_TYPE_16) {
        return FAT_ERR_FAT16;
    } else {
        fat_bytes_to_int_le(&fat_dat[0x24], &fat_sectors_per_fat);
        fat_clust_num = fat_sectors_per_fat * 128;
    }

    fat_reserved_sectors = fat_bytes_to_short(&fat_dat[0x0E]);
    fat_cluster_size = fat_dat[0x0D];
    fat_root_entry = fat_sectors_per_fat * 2 + fat_pbr_entry + fat_reserved_sectors;
    fat_entry = fat_pbr_entry + fat_reserved_sectors;
    fat_data_entry = fat_root_entry;

    resp = fat_cluster_size;
    fat_cluster_size_pow = 0;

    while (resp > 1) {
        resp >>= 1;
        fat_cluster_size_pow++;
    }

    fat_first_free_clust = 2;



    return 0;
}

u8 fat_cache_read_dat(u32 sector) {

    if (sector == fat_dat_sector)return 0;
    fat_dat_sector = sector;
    return diskReadToRam(sector, fat_dat, 1);

}

u8 fat_cache_read_tab(u32 sector) {


    if (sector == fat_tab_sector)return 0;
    fat_tab_sector = sector;
    sector += fat_entry;

    return diskReadToRam(sector, fat_tab, 1);

}

u8 fat_get_type() {
    if (str_cmp_len(&fat_dat[0x36], "FAT", 3) != 0)return FAT_TYPE_16;
    if (str_cmp_len(&fat_dat[0x52], "FAT", 3) != 0)return FAT_TYPE_32;
    return 0;
}

u16 fat_bytes_to_short(u8 *src) {

    return *src | (src[1] << 8);
}

u8 fat_load_dir(u32 entry) {

    u8 resp;
    u32 sector;
    u32 sector_32;
    u8 lfn_record;
    u8 *ptr;
    u8 i;
    u8 u;
    u8 first_char;
    u8 fat16_root_ctr;

    fat16_root_ctr = 0;
    lfn_record = 0;
    first_char = 0;
    fat_dir_size = 0;


    if (entry == 0) {
        sector = fat_root_entry;
        entry = 2;
        fat_dir_is_root = 1;
        fat_sub_dir_clust = 0;
    } else {
        fat_dir_is_root = 0;
        sector = fat_cluster_to_sector(entry);

        resp = fat_cache_read_dat(sector);
        if (resp)return resp;

        fat_sub_dir_clust = fat_bytes_to_short(&fat_dat[0x34]);
        fat_sub_dir_clust <<= 16;
        fat_sub_dir_clust |= fat_bytes_to_short(&fat_dat[0x3a]);

    }



    for (;;) {

        for (u = 0; u < fat_cluster_size; u++) {


            sector_32 = (u32) sector << 4;
            resp = fat_cache_read_dat(sector);
            if (resp)return resp;
            ptr = fat_dat;

            for (i = 0; i < 16; i++, ptr += 32) {

                if (*ptr == 0)return 0;
                if (*ptr == 0xe5 || *ptr == 0x2e) {
                    lfn_record = 0;
                    continue;
                }

                //lfn chunk
                if (ptr[0x0B] == 0x0F) {

                    first_char = ptr[1];

                    if ((*ptr & 0xF0) == 0x40) {
                        lfn_record = 1;
                        fat_dir[fat_dir_size] = sector_32 | i;
                    }

                    continue;
                }

                if ((first_char == (u8) '.' && lfn_record) || (ptr[0x0B] & 0x0A) != 0) {
                    lfn_record = 0;
                    continue;
                }

                if (lfn_record == 0) {
                    fat_dir[fat_dir_size] = sector_32 | i;
                }

                //if ((ptr[0x0B] & 0x10) == 0x10) sector_32 |= 0x10;

                fat_dir_size++;
                lfn_record = 0;
                if (fat_dir_size == OS_MAX_DIR_SIZE)return 0;
            }

            sector++;

        }


        resp = fat_get_next_cluster(&entry);
        if (resp)return resp;


        sector = fat_cluster_to_sector(entry);


    }


}

u32 fat_cluster_to_sector(u32 cluster) {

    static u32 val;
    val = cluster - 2;
    val <<= fat_cluster_size_pow;
    val += fat_data_entry;

    return val;

    //return (u32) (cluster - 2) * (u32) fat_cluster_size + (u32) fat_data_entry;
}

u32 fat_sector_to_cluster(u32 sector) {


    static u32 val;
    val = sector - fat_data_entry;
    val >>= fat_cluster_size_pow;
    val += 2;

    return val;

    //return (((u32) sector - (u32) fat_data_entry) >> fat_cluster_sdiv) + 2;
}

u8 fat_in_cluster_addr(u32 sector) {

    sector -= fat_data_entry;
    return sector & (fat_cluster_size - 1);
}

u8 fat_get_table_record(u32 cluster, u32 *val) {

    u32 table_sector;
    u8 resp;
    u8 *ptr;

    table_sector = cluster / 128;
    resp = fat_cache_read_tab(table_sector);
    if (resp)return resp;

    ptr = &fat_tab[(cluster & 127) * 4];
    fat_bytes_to_int_le(ptr, val);


    return 0;

}

u8 fat_get_next_cluster(u32 *cluster) {

    u8 resp;

    resp = fat_get_table_record(*cluster, cluster);
    //if (resp)return resp;

    return resp;
}

u8 fat_get_full_record(u32 hdr_entry, FatFullRecord *rec) {


    u8 *name_ptr;
    u8 *ptr;
    u8 resp;
    u8 i;
    u8 *rec_name = (u8 *) rec->name;

    //u8 is_dir = entry & 0x10;
    u8 rec_idx = hdr_entry & 0x0f;
    u32 sector = (u32) hdr_entry >> 4;
    u32 cluster = fat_sector_to_cluster(sector);
    u8 in_cluster = fat_in_cluster_addr(sector);
    const u8 *lfn_struct_ptr;

    ptr = (u8 *) & fat_dat[rec_idx * 32];

    rec_name[0] = 0;
    rec_name[208] = 0;
    rec->hdr_entry = hdr_entry;


    for (;;) {

        resp = fat_cache_read_dat(sector);
        if (resp)return resp;


        for (; rec_idx < 16; rec_idx++) {
            if (ptr[0x0B] != 0x0F)break;

            name_ptr = (u8 *) & rec_name[((*ptr - 1) & 0x0f) * 13];
            if ((*ptr & 0x40) == 0x40)name_ptr[13] = 0;

            lfn_struct_ptr = (u8*) lfn_char_struct;
            for (i = 0; i < 13; i++)*name_ptr++ = ptr[*lfn_struct_ptr++];

            ptr += 32;
        }
        if (rec_idx != 16)break;

        rec_idx = 0;
        ptr = (u8 *) fat_dat;
        in_cluster++;
        sector++;

        if (in_cluster == fat_cluster_size) {
            //if (!skip_name)name[0] = 0;
            resp = fat_get_next_cluster(&cluster);
            if (resp)return resp;
            //cluster++;
            sector = fat_cluster_to_sector(cluster);
            in_cluster = 0;
        }
    }


    if (rec_name[0] == 0) {
        for (i = 0; i < 11; i++)rec_name[i] = ptr[i];
        rec_name[i] = 0;
    }

    rec->hdr_entry_8 = (sector << 4) | rec_idx;

    rec->is_dir = ptr[0x0B] & 0x10;


    rec->data_clsut = fat_bytes_to_short(&ptr[0x14]);
    rec->data_clsut <<= 16;
    rec->data_clsut |= fat_bytes_to_short(&ptr[0x1a]);


    fat_bytes_to_int_le(&ptr[0x1c], &rec->size);


    return 0;

}

u8 fat_open_file(FatFullRecord *rec, u16 wr_slen) {

    u8 resp;

    if (rec->is_dir)return FAT_ERR_NOT_FILE;

    file.size = rec->size;
    file.sec_available = file.size / 512;
    if ((file.size & 511) != 0)file.sec_available++;

    if (wr_slen > file.sec_available) {
        resp = fat_resize_file(rec, wr_slen);
        if (resp)return resp;
        resp = fat_get_full_record(rec->hdr_entry, rec);
        if (resp)return resp;
    }


    file.cluster = rec->data_clsut;
    file.sector = fat_cluster_to_sector(file.cluster);
    file.in_cluster = 0;
    file.size = rec->size;
    file.wr_slen = wr_slen ? 1 : 0;
    file.sec_available = file.size / 512;
    if ((file.size & 511) != 0)file.sec_available++;


    return 0;
}

u8 fat_skip_sectors(u16 slen) {

    u8 resp;
    if (file.sec_available < slen)return FAT_ERR_OUT_OF_FILE;
    file.sec_available -= slen;

    while (slen) {

        file.sector++;
        file.in_cluster++;
        slen--;

        if (file.in_cluster == fat_cluster_size) {
            file.in_cluster = 0;
            resp = fat_get_next_cluster(&file.cluster);
            if (resp)return resp;
            file.sector = fat_cluster_to_sector(file.cluster);
        }

    }

    return 0;
}

u8 fat_read(u8 *dst, u16 slen, u8 memory) {

    u8 resp;
    u16 len;
    if (file.wr_slen)return FAT_ERR_FILE_MODE1;
    if (file.sec_available < slen)return FAT_ERR_OUT_OF_FILE;
    file.sec_available -= slen;

    while (slen) {


        if (file.in_cluster == 0) {
            len = slen > fat_cluster_size ? fat_cluster_size : slen;
        } else {
            len = fat_cluster_size - file.in_cluster;
            if (len > slen)len = slen;
        }

        if (memory == ROM) {
            resp = diskReadToRom(file.sector, dst, len);
            *((u16 *) dst) = *((u16 *) dst) + len;
        } else {
            resp = diskReadToRam(file.sector, dst, len);
            dst += len * 512;
        }
        if (resp)return resp;

        file.in_cluster += len;

        slen -= len;

        if (file.in_cluster == fat_cluster_size) {
            file.in_cluster = 0;
            resp = fat_get_next_cluster(&file.cluster);
            if (resp)return resp;
            file.sector = fat_cluster_to_sector(file.cluster);
            if (resp)return resp;
        } else {
            file.sector += len;
        }
    }

    return 0;
}

void fat_make_path_name(u8 *dst, u8 *dir, u8 *filename) {

    dst[0] = 0;
    str_append(dst, dir);
    if (str_length(dir) != 1)str_append(dst, "/");
    str_append(dst, filename);
}

void fat_make_sync_name(u8 *dir, u8 *filename, u8 *ext, u8 *dst) {

    u8 ext_exist = 0;
    u8 *ptr;
    dst[0] = 0;

    fat_make_path_name(dst, dir, filename);
    ptr = dst;

    while (*ptr != 0) {
        if (*ptr == (u8) '.')ext_exist = 1;
        ptr++;
    }

    if (ext_exist) {
        while (*ptr != (u8) '.')ptr--;
    }

    if (*ext != (u8) '.')*ptr++ = (u8) '.';
    *ptr = 0;

    str_append(dst, ext);

}

u8 fat_open_file_by_name(u8 *name, FatFullRecord *rec, u16 wr_slen) {

    u8 resp;
    u32 file_root;

    resp = fat_seek_record(name, rec, &file_root, 0);
    if (resp == FAT_ERR_NOT_EXIST) {
        if (wr_slen == 0)return FAT_ERR_NOT_EXIST;
        if (file_root == FAT_END_VAL)return FAT_ERR_PATH_NOT_EXIST;
        resp = fat_make_record(name, file_root, wr_slen, 0);
        if (resp)return resp;

        resp = fat_seek_record(name, rec, &file_root, 0);
        if (resp)return resp;
    }

    resp = fat_open_file(rec, wr_slen);

    return resp;
}

u8 fat_seek_record(u8 *name, FatFullRecord *rec, u32 *file_root, u8 dir) {

    u8 resp = 0;
    u8 sub_name_len;
    u8 last_sub = 0;
    u32 dir_addr = 0;
    u8 exist = 0;
    u16 i;
    u8 u;
    u8 str_empty;

    *file_root = FAT_END_VAL;

    for (;;) {

        if (*name == (u8) '/')name++;
        sub_name_len = 0;
        while (name[sub_name_len] != (u8) '/' && name[sub_name_len] != 0)sub_name_len++;
        if (name[sub_name_len] == 0)last_sub = 1;



        resp = fat_load_dir(dir_addr);
        if (resp)return resp;

        for (i = 0; i < fat_dir_size; i++) {
            resp = fat_get_full_record(fat_dir[i], rec);
            if (resp)return resp;
            if (!rec->is_dir && !last_sub)continue;

            //check if name end is empty. in case if file name lenght bigger than subname
            str_empty = 1;
            for (u = sub_name_len; rec->name[u] != 0; u++) {
                if (rec->name[u] != (u8) ' ') {
                    str_empty = 0;
                    break;
                }
            }

            if (str_empty && str_cmp_len(name, rec->name, sub_name_len)) {
                if (last_sub) {
                    if (rec->is_dir && !dir)continue;
                    if (!rec->is_dir && dir)continue;
                    exist = 1;
                } else {
                    dir_addr = rec->data_clsut;
                }
                name += sub_name_len;
                break;
            }

        }

        if (resp || last_sub || i == fat_dir_size)break;
    }

    if (last_sub)*file_root = dir_addr;
    if (!exist)return FAT_ERR_NOT_EXIST;


    return 0;
}

void fat_bytes_to_int_le(u8 *src, u32 *dst) {

    
#ifdef MOTO_ORDER
    ((u8 *) dst)[3] = *src++;
    ((u8 *) dst)[2] = *src++;
    ((u8 *) dst)[1] = *src++;
    ((u8 *) dst)[0] = *src++;
#else
    *dst = *(u32 *) src;
#endif
    
}

void fat_int_to_bytes_le(u32 src, u8 *dst) {

#ifdef MOTO_ORDER
    *dst++ = src & 0xff;
    *dst++ = (src >> 8) & 0xff;
    *dst++ = ((u32) src >> 16) & 0xff;
    *dst++ = ((u32) src >> 24) & 0xff;
#else    
    *(u32 *) dst = src;
#endif
    
}


//****************************************************************************** wr section
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************

u8 fat_seek_free_cluster(u32 *base_clust, u16 clen) {

    u32 clust;
    u8 resp;
    u16 ctr;
    u32 sector;
    u32 *ptr32;

    u32 tab_val;

    ptr32 = (u32 *) & fat_tab[508];
    sector = fat_first_free_clust / 128;



    for (;;) {

        /*gConsPrint("seek...");
        gRepaint();*/

        for (;;) {

            resp = fat_cache_read_tab(sector);
            if (resp)return resp;

            if (*ptr32 == 0)break;

            sector += FAT_SEEK_STEP;
            if (sector >= fat_sectors_per_fat) return FAT_ERR_NO_FRE_SPACE;

        }

        /*gConsPrint("ok: ");
        gAppendHex32(sector);
        gRepaint();
        sysJoyWait();*/


        clust = sector * 128;
        if (clust < 2)clust = 2;

        ctr = 0;
        for (;;) {

            resp = fat_get_table_record(clust, &tab_val);
            if (resp)return resp;
            clust++;

            if (tab_val == 0) {

                ctr++;
                if (ctr == clen) {
                    *base_clust = clust - clen;
                    fat_first_free_clust = *base_clust;
                    return 0;
                }

            } else {

                if (ctr != 0) break;
            }

        }

        sector++;
    }




}

u8 fat_cache_save_tab() {

    u8 resp;
    u32 sector;

    sector = fat_tab_sector + fat_entry;

    resp = diskWrite(sector, fat_tab, 1);
    if (resp)return resp;

    resp = diskWrite(sector + fat_sectors_per_fat, fat_tab, 1);
    //if (resp)return resp;

    return resp;
}

u8 fat_cache_save_data() {

    return diskWrite(fat_dat_sector, fat_dat, 1);

}

u8 fat_catch_clusters(u32 base, u16 clen) {

    u8 resp;
    u32 sector;
    u32 *ptr;


    for (;;) {

        sector = base / 128;
        resp = fat_cache_read_tab(sector);
        if (resp)return resp;
        ptr = (u32 *) & fat_tab[(base & 127) * 4];

        for (;;) {

            base++;
            clen--;
            if (clen == 0) {
                fat_int_to_bytes_le(FAT_END_VAL, (u8*) ptr);
                resp = fat_cache_save_tab();
                return resp;
            } else {
                fat_int_to_bytes_le(base, (u8*) ptr);
                ptr++;
            }

            if ((base & 127) == 0) {
                resp = fat_cache_save_tab();
                if (resp)return resp;
                break;
            }

        }
    }



}

u8 fat_resize_file(FatFullRecord *rec, u16 slen) {

    u8 resp;
    u32 new_size;
    u32 hdr_sector;
    u8 hdr_idx;
    u8 *ptr;
    u16 old_clen;
    u16 new_clen;
    u16 old_slen;
    u32 end_cluster;

    new_size = (u32) slen * 512;

    

    if (new_size <= rec->size)return FAT_ERR_SIZE_EQ;
    hdr_idx = (u8) (rec->hdr_entry_8 & 0x0F);
    hdr_sector = rec->hdr_entry_8 >> 4;
    resp = fat_cache_read_dat(hdr_sector);
    if (resp)return resp;

    

    ptr = (u8 *) & fat_dat[(u16) hdr_idx * 32];


    fat_int_to_bytes_le(new_size, &ptr[0x1c]);


    resp = fat_cache_save_data();
    if (resp)return resp;


    old_slen = rec->size / 512;
    if ((rec->size & 511) != 0)old_slen++;

    //old_clen = old_slen / fat_cluster_size;
    old_clen = old_slen >> fat_cluster_size_pow;
    if ((old_slen & (fat_cluster_size - 1)) != 0)old_clen++;

    //new_clen = slen / fat_cluster_size;
    new_clen = slen >> fat_cluster_size_pow;
    if ((slen & (fat_cluster_size - 1)) != 0)new_clen++;


    if (old_clen == new_clen)return 0;

    new_clen -= old_clen;


    resp = fat_seek_rec_end(rec->data_clsut, &end_cluster);
    if (resp)return resp;


    return fat_expand(end_cluster, new_clen, 0);


}

u8 fat_seek_rec_end(u32 start_clust, u32 *end_clust) {

    u8 resp;

    while (start_clust != FAT_END_VAL) {
        *end_clust = start_clust;
        resp = fat_get_next_cluster(&start_clust);

        if (resp)return resp;
    }

    return 0;
}

u8 fat_join(u32 last_clust, u32 next_clust) {

    u32 val;
    u8 *ptr;
    u8 resp;

    resp = fat_get_table_record(last_clust, &val);
    if (resp)return resp;

    if (val != FAT_END_VAL)return FAT_ERR_NOT_LAST;

    ptr = (u8 *) & fat_tab[(last_clust & 127) * 4];
    fat_int_to_bytes_le(next_clust, ptr);

    return fat_cache_save_tab();
}

u8 fat_write_file(u8 *src, u16 slen) {


    u8 resp;
    u16 len;
    if (!file.wr_slen)return FAT_ERR_FILE_MODE2;
    if (file.sec_available < slen)return FAT_ERR_OUT_OF_FILE_WR;
    file.sec_available -= slen;
    if (file.sec_available == 0)file.wr_slen = 0;

    while (slen) {

        if (file.in_cluster == 0) {
            len = slen > fat_cluster_size ? fat_cluster_size : slen;
        } else {
            len = fat_cluster_size - file.in_cluster;
            if (len > slen)len = slen;
        }


        resp = diskWrite(file.sector, src, len);
        if (resp)return resp;

        file.in_cluster += len;
        src += len * 512;
        slen -= len;

        if (file.in_cluster == fat_cluster_size) {
            file.in_cluster = 0;
            resp = fat_get_next_cluster(&file.cluster);
            if (resp)return resp;
            file.sector = fat_cluster_to_sector(file.cluster);
            if (resp)return resp;
        } else {
            file.sector += len;
        }
    }


    return 0;
}

u8 fat_expand(u32 last_clust, u16 csize, u8 clean) {

    u8 resp;
    u32 free_clust;


    resp = fat_seek_free_cluster(&free_clust, csize);
    if (resp)return resp;

    resp = fat_catch_clusters(free_clust, csize);
    if (resp)return resp;


    resp = fat_join(last_clust, free_clust);
    if (resp)return resp;

    if (clean) {

        resp = fat_clean_cluster(free_clust, csize);
        if (resp)return resp;
    }


    return 0;
}

u8 fat_clean_cluster(u32 cluster, u16 clen) {

    u32 sector;
    u8 resp;
    u8 i;

    sector = fat_cluster_to_sector(cluster);
    fat_dat_sector = ~0;
    sysMemSet(fat_dat, 0, 512);

    while (clen--) {
        for (i = 0; i < fat_cluster_size; i++) {
            resp = diskWrite(sector, fat_dat, 1);
            if (resp)return resp;
            sector++;
        }
    }

    return 0;

}

/*
typedef struct {
    u8 name[8];
    u8 ext[3];
    u8 attrib;
    u8 x[8];
    u16 cluster_hi;
    u16 time;
    u16 date;
    u16 cluster_lo;
    u32 size;
} FatRecordHdr;*/

u8 fat_make_hdr(u8 *name, u8 rec_idx, u8 attrib, u32 entry, u32 size) {

    u8 i;
    u8 *ptr;
    u16 date;
    u16 time;

    date = osGetDate();
    time = osGetTime();

    ptr = (u8 *) & fat_dat[(u16) rec_idx * 32];

    for (i = 0; i < 11; i++)*ptr++ = name[i];
    *ptr++ = attrib;
    for (i = 0; i < 8; i++)ptr++;

    *ptr++ = (entry >> 16) & 0xff;
    *ptr++ = (entry >> 24) & 0xff;

    *ptr++ = time & 0xff; //time
    *ptr++ = time >> 8;

    *ptr++ = date & 0xff; //date
    *ptr++ = date >> 8;

    *ptr++ = entry & 0xff;
    *ptr++ = (entry >> 8) & 0xff;

    fat_int_to_bytes_le(size, ptr);

    return fat_cache_save_data();

}

void fat_make_lfn(u8 *name, u8 rec_idx, u8 lfn_idx, u8 crc) {

    u8 *dat_ptr;
    u8 *lfn_ptr;
    u8 i;
    u8 idx;
    u8 mask = 0;

    dat_ptr = (u8 *) & fat_dat[(u16) rec_idx * 32];
    lfn_ptr = (u8 *) & name[lfn_idx * 13];

    sysMemSet(dat_ptr, 0, 32);

    for (i = 0; i < 13; i++) {
        idx = lfn_char_struct[i];
        dat_ptr[idx] = *lfn_ptr++ | mask;
        dat_ptr[idx + 1] = mask;
        if (dat_ptr[idx] == 0)mask = 0xff;
    }

    if (mask != 0 || *lfn_ptr == 0)mask = 0x40;

    dat_ptr[0x00] = (lfn_idx + 1) | mask;
    dat_ptr[0x0B] = 0x0F;
    dat_ptr[0x0D] = crc;

}

u8 fat_next_dir_sector(u32 *clust, u32 *sector, u8 *in_clust) {

    u8 resp;
    u32 last_clust;



    if (*sector == 0) {

        *in_clust = 0;
        *sector = fat_cluster_to_sector(*clust);

    } else {

        *in_clust = *in_clust + 1;
        *sector = *sector + 1;

        if (*in_clust == fat_cluster_size) {
            *in_clust = 0;
            last_clust = *clust;
            resp = fat_get_next_cluster(clust);
            if (resp)return resp;

            if (*clust == FAT_END_VAL) {
                resp = fat_expand(last_clust, 1, 1);
                if (resp)return resp;
                *clust = last_clust;
                resp = fat_get_next_cluster(clust);
                if (resp)return resp;
            }

            *sector = fat_cluster_to_sector(*clust);
        }
    }


    resp = fat_cache_read_dat(*sector);
    //if (resp)return resp;

    return resp;
}

u8 fat_make_dos_name(u8 *lfn_name, u8 *dos_name, u16 idx) {

    u8 i;
    u8 dot;
    u8 crc;

    sysMemCopy(lfn_name, dos_name, 8);

    dos_name[8] = (u8) ' ';
    dos_name[9] = (u8) ' ';
    dos_name[10] = (u8) ' ';
    dot = 0;

    for (i = 0; lfn_name[i] != 0; i++) {
        if (lfn_name[i] == (u8) '.')dot = i;
    }

    if (dot != 0) {
        dot++;

        for (i = 0; i < 3; i++) {
            if (lfn_name[i] == 0)break;
            dos_name[i + 8] = lfn_name[i + dot];
        }
    }

    for (i = 0; i < 11; i++) {
        if (dos_name[i] >= (u8) 'a' && dos_name[i] <= (u8) 'z')dos_name[i] &= ~0x20;
    }


    dos_name[4] = '~';
    dos_name[5] = idx % 1000 / 100 + (u8) '0';
    dos_name[6] = idx % 100 / 10 + (u8) '0';
    dos_name[7] = idx % 10 + (u8) '0';

    crc = 0;
    for (i = 0; i < 11; i++)crc = (((crc & 1) << 7) | ((crc & 0xfe) >> 1)) + dos_name[i];

    return crc;

}

u8 fat_make_record(u8 *name, u32 dir_entry, u16 slen, u8 is_dir) {

    u32 entry;
    u32 sector;
    u16 rec_idx;
    u16 clen;
    u8 resp;
    u8 str_len;
    u8 len32;
    u8 in_clust;
    u8 i;
    u8 crc = 0;
    u8 attrib;
    u8 dos_name[11];
    u8 new_dos_name[12];
    u8 *name_ptr;
    u8 *ptr;



    name_ptr = name;
    while (*name != 0) {
        if (*name == (u8) '/')name_ptr = name;
        name++;
    }
    if (*name_ptr == (u8) '/')name_ptr++;


    if (dir_entry < 2)dir_entry = 2;

    str_len = str_length(name_ptr);
    if (str_len < 11) {

        sysMemSet(new_dos_name, ' ', 11);
        new_dos_name[11] = 0;
        for (i = 0; name_ptr[i] != 0; i++)new_dos_name[i] = name_ptr[i];
        name_ptr = (u8 *) new_dos_name;
        str_len = 11;

        //return FAT_ERR_BAD_NAME;
    }

    entry = 0;

    clen = slen / fat_cluster_size;
    if (slen % fat_cluster_size != 0)clen++;
    resp = fat_seek_free_cluster(&entry, clen);
    if (resp)return resp;
    resp = fat_catch_clusters(entry, clen);
    if (resp)return resp;

    if (is_dir) {
        resp = fat_clean_cluster(entry, 1);
        if (resp)return resp;
    }


    len32 = 0;
    if (str_len == 11) {
        sysMemCopy(name_ptr, dos_name, 11);
    }
    if (str_len > 11) {
        len32 = str_len / 13;
        if (str_len % 13 != 0)len32++;

        crc = fat_make_dos_name(name_ptr, dos_name, fat_dir_size);
    }


    attrib = is_dir ? 0x10 : 0x20;

    sector = 0;
    ptr = (u8 *) & fat_dat[512 - 32];

    for (;;) {
        resp = fat_next_dir_sector(&dir_entry, &sector, &in_clust);
        if (resp)return resp;

        if (*ptr != 0)continue;

        for (rec_idx = 0; rec_idx < 512; rec_idx += 32) {
            if (fat_dat[rec_idx] == 0)break;
        }
        if (rec_idx != 512)break;
    }


    rec_idx /= 32;
    for (i = 0; i < len32; i++) {

        fat_make_lfn(name_ptr, rec_idx, len32 - i - 1, crc);
        rec_idx++;
        if (rec_idx == 16) {
            rec_idx = 0;
            resp = fat_cache_save_data();
            if (resp)return resp;
            resp = fat_next_dir_sector(&dir_entry, &sector, &in_clust);
            if (resp)return resp;
        }
    }

    if (is_dir)slen = 0;
    resp = fat_make_hdr(dos_name, rec_idx, attrib, entry, (u32) slen * 512);
    //if (resp)return resp;

    return resp;


}

u8 fat_make_dir(FatFullRecord *rec, u8 *name) {

    u8 resp;
    u32 root;
    u32 sector;



    resp = fat_seek_record(name, rec, &root, 1);


    if (resp == 0)return FAT_ERR_EXIST;
    if (resp == FAT_ERR_NOT_EXIST) {
        if (root == FAT_END_VAL)return FAT_ERR_PATH_NOT_EXIST;
        resp = fat_make_record(name, root, fat_cluster_size, 1);
        if (resp)return resp;

        resp = fat_seek_record(name, rec, &root, 1);
        if (resp == FAT_ERR_NOT_EXIST)return FAT_ERR_DIR_MAKE;
        if (resp)return resp;
        sector = fat_cluster_to_sector(rec->data_clsut);
        resp = fat_cache_read_dat(sector);
        if (resp)return resp;


        fat_make_hdr("           ", 0, 0x10, rec->data_clsut, 0);
        fat_make_hdr("           ", 1, 0x10, root, 0);
        fat_dat[0x00] = 0x2E;
        fat_dat[0x20] = 0x2E;
        fat_dat[0x21] = 0x2E;

        resp = fat_cache_save_data();
        if (resp)return resp;

    }
   //if (resp)return resp;


    return resp;
}
