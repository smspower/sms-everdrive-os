
#include "types.h"
#include "menu_form.h"
#include "select_file.h"
#include "fat.h"
#include "vdp.h"
#include "ram_app.h"
#include "everdrive.h"
#include "prog.h"
#include "mmc.h"
#include "joy.h"
#include "sram_manager.h"

void updateOS();
void about();
void deviceInfo();

enum {
    TBOX_SRAM_MANAGER = 0,
    TBOX_DEV_INFO,
    TBOX_UPDATE_OS,
    TBOX_ABOUT,
    TBOX_SIZE
};

u8 *tbox_menu[TBOX_SIZE];

void toolBox() {

    u8 selector = 0;
    tbox_menu[TBOX_SRAM_MANAGER] = (u8 *) "save/load sram";
    tbox_menu[TBOX_DEV_INFO] = (u8 *) "device info";
    tbox_menu[TBOX_UPDATE_OS] = (u8 *) "update os";
    tbox_menu[TBOX_ABOUT] = (u8 *) "about";

    for (;;) {
        selector = menuFormShow(tbox_menu, TBOX_SIZE, 2, selector, 0);
        if (selector >= 64)return;

        if (selector == TBOX_UPDATE_OS)updateOS();
        if (selector == TBOX_SRAM_MANAGER)sramManager();
        if (selector == TBOX_DEV_INFO)deviceInfo();
        if (selector == TBOX_ABOUT)about();
    }

}

void updateOS() {

    u16 i;
    u8 resp;
    u8 idx;
    u16 u;
    u8 *ptr;
    idx = selectFile();
    if (idx == 255)return;
    vdpSetCons(2, 2);
    vdp_clean_screen();
    vdp_repaint();
    if (dir.records[idx].size != 32768) {
        vdpDrawErr("wrong file size", 0);
        return;
    }

    fatOpenFile(&dir.records[idx]);
    mmcRdBlock(file.addr_buff, mmc_buff);
    for (i = 0; i < 4; i++) {
        if (mmc_buff[128 + i] != ((u8 *) "EDOS")[i]) {
            vdpDrawErr("wrong file", 0);
            return;
        }
    }

    vdpDrawConsStr("are you sure?", PAL1);
    vdpDrawConsStr("(left)NO, (right)YES", PAL1);
    vdp_repaint();
    for (;;) {
        joy_wait_read();
        if ((joy & KEY_LEFT))return;
        if ((joy & KEY_RIGHT))break;
    }


    vdpDrawConsStr("update...", PAL1);
    vdp_repaint();
    fatOpenFile(&dir.records[idx]);
    eprErase(0xf0000, 1);
    r_in_bank_addr = 32768;
    mmcSetArg(file.addr_buff);
    BANK_CFG2_REG = 0;
    *(u8 *) 0xffff = 60;


    *((u8 *) 0x8aaa) = 0xaa;
    *((u8 *) 0x8555) = 0x55;
    *((u8 *) 0x8aaa) = 0x20;

    for (i = 0; i < 32768; i += 512) {
        if (i == 16384) {
            r_in_bank_addr = 0x8000;
            *(u8 *) 0xffff = 61;
        }
        resp = mmcBeginRead();
        if (resp != 0) {
            vdpDrawErr("mmc read error: ", resp);
            return;
        }
        r_prog_block_sd();
        mmcFinishReadInc();
        r_in_bank_addr += 512;

    }
    *((u8 *) 0x8000) = 0x90;
    *((u8 *) 0x8000) = 0x00;

    vdpDrawConsStr("verify...", PAL1);
    vdp_repaint();

    ptr = (u8 *) 32768;
    *(u8 *) 0xffff = 60;
    for (i = 0; i < 16384; i += 512) {
        if (i == 16384) {
            *(u8 *) 0xffff = 61;
            ptr = (u8 *) 32768;
        }
        //mmcRdBlock(file.addr_buff, mmc_buff);
        resp = fatReadNextBlock();
        if (resp != 0) {
            vdpDrawErr("mmc read error: ", resp);
            return;
        }
        for (u = 0; u < 512; u++) {
            if (mmc_buff[u] != *ptr++) {
                vdpDrawErr("verify error ", 0);

                return;
            }

        }

        //file.addr_buff += 512;
    }

    fatOpenFile(&dir.records[idx]);

    r_update_os_sd();


}

void about() {

    u8 *inf_ptr = (u8 *) 0xbf10;
    u8 y = 2;
    BANK_CFG2_REG = 0;
    *(u8 *) 0xffff = 3;
    vdp_clean_screen();

    if (gg_mode) {
        y = 4;
    }

    if (IS_GG_CART) {
        vdpDrawStrCenter("EverDrive GG", y, PAL1);
    } else {

        vdpDrawStrCenter("Master EverDrive", y, PAL1);
    }
    y += 2;

    if (gg_mode) {
        vdpDrawStrCenter("developed by:", y, PAL1);
        y++;
        vdpDrawStrCenter("Igor Golubovskiy", y, PAL1);
    } else {
        vdpDrawStrCenter("developed by I. Golubovskiy", y, PAL1);
    }

    y += 2;

    vdpDrawStrCenter("support:", y++, PAL1);
    vdpDrawStrCenter("http://krikzz.com", y++, PAL0);
    vdpDrawStrCenter("biokrik@gmail.com", y++, PAL0);
    y++;

    vdpDrawStrCenter("assembly date:", y++, PAL1);
    vdpDrawStrCenter(inf_ptr, y++, PAL0);
    y++;
    inf_ptr += 16;
    vdpDrawStrCenter("distributed by:", y++, PAL1);
    vdpDrawStrCenter(inf_ptr, y++, PAL0);
    inf_ptr += 32;
    if (gg_mode) {
        inf_ptr += 7;
    }
    vdpDrawStrCenter(inf_ptr, y++, PAL0);

    vdp_repaint();
    joy_wait_read();
}

void deviceInfo() {

    u8 y;
    u8 x;
    vdp_clean_screen();

    if (gg_mode) {
        x = 15;
        y = 4;
    } else {
        x = 14;
        y = 2;
    }

    vdpDrawStrRt("device:", x, y, PAL0);
    if (IS_GG_CART) {
        vdpDrawStr("everdrive gg", x + 1, y, PAL1);
    } else {
        if (gg_mode) {
            vdpDrawStr("master evd", x + 1, y, PAL1);
        } else {
            vdpDrawStr("master everdrive", x + 1, y, PAL1);
        }
    }
    y += 2;

    vdpDrawStrRt("OS:", x, y, PAL0);
    vdpDrawNum("", OS_VERSION, x + 1, y, PAL1);
    y += 2;

    vdpDrawStrRt("firmware:", x, y, PAL0);
    vdpDrawNum("", FIRM_VERSION, x + 1, y, PAL1);
    y += 2;

    vdpDrawStrRt("fat:", x, y, PAL0);
    vdpDrawStr("FAT16", x + 1, y, PAL1);
    y += 2;

    vdpDrawStrRt("dir size:", x, y, PAL0);
    vdpDrawNum("", FAT_DIR_SIZE, x + 1, y, PAL1);
    y += 2;

    vdp_repaint();
    joy_wait_read();

}