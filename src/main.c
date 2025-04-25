/*
 * File:   main.c
 * Author: krikzz
 *
 * Created on 23.06.2010
 */


#include "fat.h"
#include "os.h"
#include "sys.h"
#include "str.h"
#include "disk.h"
#include "error.h"
#include "bios.h"



u8 browser();
//extern u8 cart_3e;

u8 tmp;
u8 bi_bank_is_empty(u8 cmp_val);
void bi_clean_ram_bank(u8 val);
void bi_set_ram_bank(u8 mask);

void acclaimDbg() {

    u16 ctr = 0;
    u16 i;
    volatile u16 *map16 = (u16 *) 0x200000;
    volatile u8 *map8 = (u8 *) 0x200000;
    //volatile u8 *eep_clk = (u8 *)0x200000;
    //volatile u8 *eep_dat = (u8 *)0x200001;





    for (;;) {
        gSetXY(0, 0);
        gConsPrint("hello:");
        gAppendNum(ctr++);

        gConsPrint("");
        gConsPrint("16-bit");


        *map16 = 0;
        gConsPrint("dat-a:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i]);
        }
        gConsPrint("dat-1:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i + 0x80000]);
        }
        gConsPrint("dat-2:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i + 0x100000]);
        }
        
        gConsPrint("dat-3:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i] & 1);
        }

        gConsPrint("");
        *map16 = 0xffff;
        gConsPrint("dat-b:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i]);
        }
        gConsPrint("dat-1:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i + 0x80000]);
        }
        gConsPrint("dat-2:");
        for (i = 0; i < 16; i++) {
            gAppendHex8(map8[i + 0x100000]);
        }

        gConsPrint("");
        gConsPrint("8-bit");

        *map8 = 0;
        gConsPrint("dat-a:");
        for (i = 0; i < 8; i++) {
            gAppendHex16(map16[i]);
        }

        *map8 = 0xff;
        gConsPrint("dat-b:");
        for (i = 0; i < 8; i++) {
            gAppendHex16(map16[i]);
        }

        osUsbListener();
        gRepaint();

    }


}

int main() {


    u8 resp;



    resp = osInit();

    acclaimDbg();

    // rtcTest();
    /*
        if (resp == DISK_ERR_INIT) {
            resp = osStartGame();
        }*/

    if (resp) {
        guiPrintError(resp);
        for (;;)osUsbListener();
    }

    //bi_update_boot();
    for (;;) {
        resp = browser();
        guiPrintError(resp);
    }


}

void vb() {
}

void hb() {
}

void in() {
}


