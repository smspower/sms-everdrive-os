
#include "types.h"
#include "mmc.h"
#include "fat.h"
#include "vdp.h"
#include "everdrive.h"
#include "joy.h"
#include "ram_app.h"
#include "prog.h"



extern u16 epr_addr;
void prog_block_sd();
extern u16 in_bank_addr;

/*
#define MEM8(addr) *((u8 *)addr)
#define MEM16(addr) *((u16 *)addr)
#define MEM32(addr) *((u32 *)addr)
 */
u8 progGame(FatRecord *rec) {

    u8 *map_cnt_ptr = (u8 *) 0xffff;
    u8 resp;
    u8 bank16k;
    u32 len;
    u8 ctr;
    u16 ctr16;
    u8 y;
    u8 x;
    vdp_clean_screen();
    vdp_repaint();
    vdpSetCons(2, 2);
    fatOpenFile(rec);


    len = rec->size;

    if ((len & 1023) == 512) {
        len -= 512;
        file.addr_buff += 512;
    }

    if (len < 65536)len = 65536;
    if (len > 0x100000) {
        vdpDrawErr("file too big", 1);
        return 1;
    }


    //vdpDrawErr("erase...", 1);
    vdpDrawConsStr("erase...", PAL1);
    vdp_repaint();
    //len /= 65536;
    len >>= 16;
    eprErase(0x100000, len);
    len <<= 16;
    //len *= 65536;
    vdpDrawConsStr("write...", PAL1);
    vdp_repaint();

    BANK_CFG2_REG = (1 << _BCFG_ROM_BANK);
    bank16k = 0;
    r_in_bank_addr = 0x8000;
    mmcSetArg(file.addr_buff);
    *map_cnt_ptr = 0;
    *((u8 *) 0x8aaa) = 0xaa;
    *((u8 *) 0x8555) = 0x55;
    *((u8 *) 0x8aaa) = 0x20;


    ctr = 0;
    ctr16 = 0;
    y = cn_y;
    if (gg_mode) {
        x = 6;
    } else {
        x = 2;
    }

    vdpDrawStr("     ", 2, y, PAL1);
    vdpDrawNum("", len >> 10, 2, y, PAL1);
    vdp_repaint();

    while (len) {

        resp = mmcBeginRead();
        if (resp != 0) {
            vdpDrawErr("mmc read error: ", resp);
            return 2;
        }
        //AUTO_BUSY_OFF;
        r_prog_block_sd();

        mmcFinishReadInc();
        len -= 512;
        r_in_bank_addr += 512;
        if (r_in_bank_addr == 0xc000) {
            r_in_bank_addr = 0x8000;
            bank16k++;
            *map_cnt_ptr = bank16k;
        }


        ctr16 += 1024;
        if (ctr16 == 0) {

            vdpDrawStr("     ", x, y, PAL1);
            vdpDrawNum("", len >> 10, x, y, PAL1);
            vdp_repaint();

        }

    }
    *((u8 *) 0x8000) = 0x90;
    *((u8 *) 0x8000) = 0x00;

    //vdpDrawConsStr("", PAL1);
    //vdpDrawConsStr("done!", PAL1);
    vdp_repaint();


    //joy_wait_read();
    return 0;
}

//AAA AA, 555 55, AAA 80, AAA AA, 555 55, BA 30

void eprErase(u32 base_addr, u8 blocks) {


    u8 *map_cnt_ptr = (u8 *) 0xffff;


    if (base_addr >= 0x100000) {
        BANK_CFG2_REG = (1 << _BCFG_ROM_BANK);
    } else {
        BANK_CFG2_REG = (0 << _BCFG_ROM_BANK);
    }

    base_addr &= 0xfffff;
    *map_cnt_ptr = (u32) base_addr / 16384;


    r_erase_len = blocks;
    r_erase();

}

