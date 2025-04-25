


#include "mmc.h"

#include "types.h"
#include "menu_form.h"
#include "vdp.h"
#include "select_file.h"
#include "fat.h"
#include "everdrive.h"


void sramSave();
void sramLoad();

enum {
    SM_LOAD = 0,
    SM_SAVE,
    SM_SIZE
};

u8 *sram_manager_items[SM_SIZE];

void sramManager() {

    u8 selector = 0;
    sram_manager_items[SM_LOAD] = (u8 *) "load from SD";
    sram_manager_items[SM_SAVE] = (u8 *) "save to SD";

    for (;;) {
        selector = menuFormShow(sram_manager_items, SM_SIZE, 2, selector, 0);
        if (selector >= 64)return;

        if (selector == SM_SAVE) {
            sramSave();
        } else if (selector == SM_LOAD) {
            sramLoad();
        }
    }

}

void sramLoad() {

    u16 u;
    u16 i;
    u8 resp;
    u8 *bcfg = (u8 *) 0xfffc;
    u8 *ram_ptr = (u8 *) 0x8000;
    u8 idx = selectFile();
    u16 len = FIFO_PORT == 1 ? 16384 : 32768;
    if (idx == 255)return;



    vdp_clean_screen();
    vdpSetCons(2, 2);
    vdp_repaint();

    if (dir.records[idx].size > len) {
        if (len == 16384) {
            vdpDrawErr("max file size 16384", 0);
        } else {
            vdpDrawErr("max file size 32768", 0);
        }
        return;
    }

    fatOpenFile(&dir.records[idx]);

    vdpDrawConsStr("copy...", PAL1);
    vdp_repaint();
    *bcfg = 1 << _BCFG_RAM_NROM;


    for (i = 0; i < len && i < dir.records[idx].size; i += 512) {
        if (i == 16384) {
            *bcfg = 1 << _BCFG_RAM_BANK | 1 << _BCFG_RAM_NROM;
            ram_ptr = (u8 *) 0x8000;
            vdpDrawConsStr("cb", PAL1);
            vdp_repaint();
        }
        resp = fatReadNextBlock();
        if (resp != 0) {
            vdpDrawErr("read error: ", resp);
            return;
        }
        for (u = 0; u < 512; u++) {
            //*ram_ptr++ = i / 512;
            *ram_ptr++ = mmc_buff[u];
        }
    }





}

void sramSave() {

    u16 i;
    u8 resp;
    u8 *ram_ptr = (u8 *) 0x8000;
    u8 *bcfg = (u8 *) 0xfffc;
    u8 idx = selectFile();
    u16 len = FIFO_PORT == 1 ? 16384 : 32768;
    if (idx == 255)return;


    vdp_clean_screen();
    vdpSetCons(2, 2);
    vdp_repaint();

    fatOpenFile(&dir.records[idx]);

    if (dir.records[idx].size < len) {
        vdpDrawErr("file size too small", 0);
        return;
    }

    vdpDrawConsStr("copy...", PAL1);
    vdp_repaint();
    *bcfg = 1 << _BCFG_RAM_NROM;

    for (i = 0; i < len; i += 512) {
        if (i == 16384) {
            *bcfg = 1 << _BCFG_RAM_BANK | 1 << _BCFG_RAM_NROM;
            ram_ptr = (u8 *) 0x8000;
            vdpDrawConsStr("cb", PAL1);
            vdp_repaint();
        }
        resp = fatWriteNextBlock(ram_ptr);
        if (resp != 0) {
            vdpDrawErr("read error: ", resp);
            return;
        }
        ram_ptr += 512;

    }
}