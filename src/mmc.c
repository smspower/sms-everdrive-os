

#include "types.h"
#include "everdrive.h"
#include "fat.h"


#define CMD0  0x40    // software reset
#define CMD1  0x41    // brings card out of idle state
#define CMD17 0x51    // read single block
#define CMD24 0x58    // writes a single block

u8 mmcCmd(u8 cmd, u32 arg);
void mmc_cmd_send();
void mmc_read();
void mmc_begin_read();
void mmc_start_read();
void mmc_finish_read();
void mmc_write_block();

extern u8 mmc_cmd;
extern u8 mmc_resp;
extern u16 mmc_buff_ptr;
extern u32 mmc_arg;

u8 mmc_buff[512];

u8 mmcInit() {

    u16 i;
    u8 dat;
    SS_OFF;
    SPI_SPEED_OFF;

    for (i = 0; i < 20; i++) {
        SPI_PORT = 0xff;
        dat = SPI_PORT;
    }

    if (mmcCmd(CMD0, 0) != 1) {
        return 1;
    }


    i = 0;
    while (mmcCmd(CMD1, 0) != 0) {
        if (i++ == 65535) {
            return 2;
        }
    }

    SS_ON;
    SPI_PORT = 0xff;
    dat = SPI_PORT;
    SS_OFF;
    SPI_SPEED_ON;

    return 0;
}

u8 mmcCmd(u8 cmd, u32 arg) {

    mmc_cmd = cmd;
    mmc_arg = arg;
    mmc_cmd_send();

    return SPI_PORT;
}

u8 mmcRdBlock(u32 addr, u8 *buff) {


    //mmcCmd(CMD17, addr);
    addr += fat_pbr_ptr;
    mmc_buff_ptr = (u16) buff;
    mmcCmd(CMD17, addr);
    mmc_begin_read();
    mmc_start_read();
    mmc_finish_read();
    //mmc_read();

    return mmc_resp;

}

void mmcSetArg(u32 arg) {
    arg += fat_pbr_ptr;
    mmc_arg = arg;
}

u8 mmcBeginRead() {

    mmcCmd(CMD17, mmc_arg);
    mmc_begin_read();
    return mmc_resp;
}

void mmcFinishReadInc() {

    mmc_finish_read();
    mmc_arg += 512;
}

u8 mmcWrBlock(u32 addr, u8 *data_ptr) {

    u8 resp;
    addr += fat_pbr_ptr;
    mmc_buff_ptr = (u16) data_ptr;
    resp = mmcCmd(CMD24, addr);
    if (resp != 0)return 1;
    mmc_write_block();
    if (mmc_resp != 0)return 2;
    return 0;

}