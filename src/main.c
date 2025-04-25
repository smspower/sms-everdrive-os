

#include "types.h"
#include "fat.h"
#include "vdp.h"
#include "menu_form.h"
#include "toolbox.h"
#include "everdrive.h"
#include "select_file.h"
#include "mmc.h"
#include "joy.h"
#include "prog.h"
#include "usb.h"
#include "ram_app.h"
//void vdp_load_font(short bob);



/*
0x7E : Reading: returns VDP V counter. Writing: writes data to Sound Chip
0x7F : Reading: returns VDP H counter. Writing: writes data to Sound Chip (mirror of above)
0xBE : Reading: reads VDP data port: Writing: writes vdp data port
0xBF : Reading: Gets VDP statis: Writing: writes to vdp control port
0xDC : Reading: Reads joypad 1. Writing: cannot write to
0xDD : Reading: Reads joypad 2. Writing: cannot write to
 */

void startGame();

#define VDP_DAT *((u8 *)0xbe)
#define VDP_CNT *((u8 *)0xbf)

enum {
    MENU_PLAY = 0,
    MENU_SELECT_GAME,
    MENU_TOOLBOX,
    MENU_SIZE
};

u8 *main_menu[MENU_SIZE];
extern u32 mmc_arg;

u8 port_cfg;

void main() {


    u8 resp;
    u8 selector = 0;

    vdp_off();
    vdp_init();

    joy = 0;



    RAM_REGS_ON;
    ROM_REGS_ON;
    CFG_REG = 1;
    AUTO_BUSY_OFF;

    //vdp_on();
    joy_read();
    if (joy == (KEY_C | KEY_LEFT) || IS_GG_CART) {
        gg_mode = 1;
    } else {
        gg_mode = 0;
    }

    if (gg_mode) {
        vdp_load_gg_pal();
    } else {
        vdp_load_sms_pal();
    }
    vdp_on();

    //BANK_CFG2_REG = 1 << _BCFG_RAM_NROM;

    //AUTO_BUSY_ON;

    usbInit();
    r_ram_app_init();
    mmc_arg = 0x12;


    main_menu[MENU_PLAY] = (u8 *) "start game";
    main_menu[MENU_SELECT_GAME] = (u8 *) "select game";
    main_menu[MENU_TOOLBOX] = (u8 *) "toolbox";

    for (;;) {
        selector = menuFormShow(main_menu, MENU_SIZE, 2, selector, 1);
        if (selector >= 64) {
            selector -= 64;
            continue;
        }

        if (selector == MENU_TOOLBOX) {
            toolBox();
            continue;
        }

        if (selector == MENU_PLAY) {
            vdp_off();
            startGame();
            continue;
        }

        if (selector == MENU_SELECT_GAME) {
            resp = selectFile();
            if (resp == 255)continue;
            resp = progGame(&dir.records[resp]);
            if (resp == 0) {
                vdp_off();
                //start_game();
                startGame();
            }


            continue;
        }
    }


}

void startGame() {

    u8 i;
    u8 *rom_ptr = (u8*) 0xbfe9;
    volatile u8 codemasters = 1;
    volatile u8 norm_size;
    volatile u8 system;
    volatile u8 *hd_ptr = (u8 *) 0xbff0;
    *((u8 *) 0xffff) = 1;
    BANK_CFG2_REG = 1 << _BCFG_ROM_BANK;

    norm_size = *((u8 *) 0xbfff);
    norm_size &= 0xf;
    system = *((u8 *) 0xbfff);
    system >>= 4;


    //norm_size = (u8) (norm_size == 0xc ? 32 : norm_size == 0xe ? 64 : norm_size == 0xf ? 128 : norm_size == 0 ? 256 : norm_size == 1 ? 512 : 1024) / 16;

    if (norm_size == 0x0c)norm_size = 2;
    else
        if (norm_size == 0x0e)norm_size = 4;
    else
        if (norm_size == 0x0f)norm_size = 8;
    else
        if (norm_size == 0x00)norm_size = 16;
    else
        if (norm_size == 0x01)norm_size = 32;
    else
        norm_size = 64;

    if (norm_size != *((u8 *) 0xbfe0) && norm_size != 2)codemasters = 0;
    /*
        vdp_on();
        vdpDrawNum("norm: ", norm_size, 2, 10, PAL1);
        vdpDrawNum("code: ", *((u8 *) 0x7fe0), 2, 11, PAL1);
        vdp_repaint();
        joy_wait_read();
     */
    if (*rom_ptr++ == 0)codemasters = 0;
    for (i = 0; i < 6; i++) {
        if (*rom_ptr++ != 0)codemasters = 0;
    }

    if (*((u8 *) 0xbfe0) < 8 || *((u8 *) 0xbfe0) > 32)codemasters = 0;

    r_start_cfg = codemasters ? 1 << _CFG_CDM_MAP : 0;
    r_start_cfg |= 1 << _CFG_ROM_WE_OFF;
    if (system < 5)r_start_cfg |= 1 << _CFG_SMS_MODE;
    if (hd_ptr[0] != 'T' || hd_ptr[1] != 'M' || hd_ptr[2] != 'R')r_start_cfg |= 1 << _CFG_SMS_MODE;

    r_start_game();
}