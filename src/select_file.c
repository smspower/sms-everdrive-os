
#include "types.h"
#include "mmc.h"
#include "vdp.h"
#include "joy.h"
#include "fat.h"

#define MENU_ITEMS_SMS 19
#define MENU_ITEMS_GG 16
u16 dir_tree[32];
u8 selector_tree[32];

u8 selectFile() {

    u8 idx_offset;
    u8 idx;
    u8 i;
    u8 resp;
    u8 page = 0;
    u8 pages_num;
    u8 selector = 0;
    u8 page_size;
    u16 dir_tree_top = 0;
    u16 key_friz = 0;
    u8 menu_items;

    if (gg_mode) {
        menu_items = MENU_ITEMS_GG;
    } else {
        menu_items = MENU_ITEMS_SMS;
    }

    vdp_clean_screen();

    vdpSetCons(2, 2);

    vdpDrawConsStr("fat init...", PAL1);
    vdp_repaint();

    resp = fatInit();
    if (resp) {
        if (resp == 1) {
            vdpDrawErr("mmc init error", resp);
            return 255;
        }

        if (resp == 2) {
            vdpDrawErr("can't read boot sector", resp);
            return 255;
        }
        if (resp == 3) {
            vdpDrawErr("unsupported fat", resp);
            return 255;
        }
        vdpDrawErr("unknown error", resp);
        return 255;
    }

    vdpDrawConsStr("open root...", PAL1);
    vdp_repaint();
    resp = fatOpenDir(0);
    if (resp) {
        vdpDrawErr("can't open root", resp);
        return 255;
    }

    pages_num = 0;


    vdp_clean_screen();
    key_friz = 0;
    for (;;) {
        pages_num = dir.size / menu_items;
        if (pages_num == 0)pages_num = 1;
        if (pages_num * menu_items < dir.size)pages_num++;


        vdpSetCons(2, 2);

        idx_offset = page * menu_items;
        page_size = dir.size - idx_offset >= menu_items ? menu_items : dir.size - idx_offset;
        selector = selector >= page_size ? page_size - 1 : selector;

        for (i = 0; i < page_size; i++) {
            if (idx_offset > dir.size)break;
            idx = dir.order[idx_offset++];

            if (selector == i) {
                vdpDrawConsStr(dir.records[idx].name, PAL1);
            } else
                if ((dir.records[idx].flags & FAT_DIR)) {
                vdpDrawConsStr(dir.records[idx].name, PAL2);
            } else {
                vdpDrawConsStr(dir.records[idx].name, PAL0);
                //vdpDrawNum("", 255, 28, cn_y, PAL2);
            }
        }
        idx_offset = page * menu_items;
        if (gg_mode) {
            vdpDrawNum("page: ", page + 1, 6, 20, PAL1);
        } else {
            vdpDrawNum("page: ", page + 1, 2, 22, PAL1);
        }
        vdp_repaint();
        //joy_read();
        //joy_wait_read();

        for (;;) {
            joy_read();
            if (joy == 0 || key_friz > 11)break;
            //PPU_WAIT_VSYNC;
            vdp_wait_vbl();
            key_friz++;
        };

        for (;;) {
            vdp_wait_vbl();
            joy_read();
            if (joy == 0)break;
            //PPU_WAIT_VSYNC;

            key_friz++;
            if (key_friz % 3 != 0)continue;
            break;
        }



        for (;;) {
            joy_read();
            if (joy != 0)break;
            key_friz = 0;
        }
        //joy_wait_read();

        if ((joy & KEY_UP)) {
            selector = selector == 0 ? page_size - 1 : selector - 1;
            continue;
        }
        if ((joy & KEY_DOWN)) {
            selector = selector == page_size - 1 ? 0 : selector + 1;
            continue;
        }

        if ((joy & KEY_RIGHT) && pages_num > 1) {
            page++;
            if (page == pages_num)page = 0;
            vdp_clean_screen();
            key_friz = 0;

            continue;
        }

        if ((joy & KEY_LEFT) && pages_num > 1) {

            page = page == 0 ? pages_num - 1 : page - 1;
            vdp_clean_screen();
            key_friz = 0;
            //selector = selector >= page_size ? page_size - 1 : selector;
            continue;
        }



        if ((joy & KEY_C) && dir_tree_top == 0) {
            return 255;
        }

        if ((joy & KEY_C) && dir_tree_top != 0) {
            vdp_clean_screen();
            vdp_repaint();
            dir_tree_top--;
            selector = selector_tree[dir_tree_top];
            page = selector / menu_items;
            selector = selector % menu_items;
            resp = fatOpenDir(dir_tree[dir_tree_top]);
            if (resp != 0) {
                vdpDrawErr("can't open dir", resp);
                return 255;
            }
            key_friz = 0;
            continue;
        }

        if ((joy & KEY_B)) {

            if (dir.size == 0)continue;
            idx = dir.order[idx_offset + selector];

            if (dir.records[idx].flags == FAT_FIL) {
                return idx;
            } else {
                vdp_clean_screen();
                vdp_repaint();
                dir_tree[dir_tree_top] = dir.entry;
                selector_tree[dir_tree_top++] = selector + idx_offset;
                resp = fatOpenDir(dir.records[idx].entry);
                if (resp != 0) {
                    vdpDrawErr("can't open dir", resp);
                    return 255;
                }
                page = 0;
                selector = 0;
                key_friz = 0;
                continue;
            }

        }
    }


}
