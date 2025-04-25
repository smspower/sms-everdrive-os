

#include "disk.h"
#include "bios.h"
#include "error.h"
#include "sys.h"

#define CMD0  0x40    /*software reset*/
#define CMD1  0x41    /*brings card out of idle state*/
#define CMD8  0x48    /*Reserved*/
#define CMD12 0x4C    /*stop transmission on multiple block read*/
#define CMD17 0x51    /*read single block*/
#define CMD18 0x52    /*read multiple block*/
#define CMD58 0x7A    /*reads the OCR register*/
#define CMD55 0x77
#define CMD41 0x69
#define CMD23 0x57
#define CMD24 0x58    /*writes a single block*/
#define CMD25 0x59    /*writes a single block*/
#define	ACMD23 (23 | 0x80)
#define SD_V2 2
#define SD_HC 1

#define DISK_CFG_SS 1
#define DISK_CFG_SPEED 2
#define DISK_CFG_16BIT 4
#define DISK_CFG_AREAD 8

#define WAIT_LEN 2048

void diskCloseRW();

extern u8 bi_card_type;
u32 disk_addr;
u8 disk_init_ok;

u8 diskCrc7(u8 *buff) {

    u16 a;
    u16 crc = 0;
    u8 len = 5;

    while (len--) {
        crc ^= *buff++;
        a = 8;
        do {
            crc <<= 1;
            if (crc & (1 << 8)) crc ^= 0x12;
        } while (--a);
    }
    return (crc & 0xfe);
}

u8 diskCmd(u8 cmd, u32 arg) {

    u16 i = 0;
    u8 resp;
    u8 crc;
    u8 buff[5];

    buff[0] = cmd;
    buff[1] = (arg >> 24) & 0xff;
    buff[2] = (arg >> 16) & 0xff;
    buff[3] = (arg >> 8) & 0xff;
    buff[4] = (arg >> 0) & 0xff;
    crc = diskCrc7(buff) | 1;

    bi_ss_on();
    bi_spi(0xff);
    bi_spi(cmd);
    bi_spi(arg >> 24);
    bi_spi(arg >> 16);
    bi_spi(arg >> 8);
    bi_spi(arg);
    bi_spi(crc);
    bi_spi(0xff);
    resp = bi_spi(0xff);

    while (resp == 0xff) {
        resp = bi_spi(0xff);
        if (i++ == 2048)break;
    }

    bi_ss_off();
    return resp;
}

u8 diskCmdFast(u8 cmd, u32 arg);

u8 diskCmdFast(u8 cmd, u32 arg) {



    u16 i = 0;
    u8 resp;
    bi_ss_on();

    bi_spi_qq(0xff);
    bi_spi_qq(cmd);
    bi_spi_qq(arg >> 24);
    bi_spi_qq(arg >> 16);
    bi_spi_qq(arg >> 8);
    bi_spi_qq(arg);
    bi_spi_qq(0xff);
    bi_spi_qq(0xff);
    resp = bi_spi_q(0xff);

    while (resp == 0xff) {
        resp = bi_spi_q(0xff);
        if (i++ == WAIT_LEN)break;
    }

    bi_ss_off();
    return resp;
}

u8 diskInit() {

    u16 i;
    u8 cmd;
    u8 resp;

#ifdef CART_GBX
    
    if (bi_card_type != 0xff) {
        bi_spi_speed_on();
        disk_addr = 1;
        disk_init_ok = 1;
        diskCloseRW();
        return 0;
    }
#endif       

    disk_init_ok = 0;
    bi_card_type = 0;
    bi_ss_off();

    diskCloseRW();

    for (i = 0; i < 32; i++)bi_spi(0xff);

    /*i = 0;
    resp = 0;
    while (resp != 1 && i++ < 2) {
        resp = diskCmd(CMD0, 0);
    }

    if (resp != 1)return DISK_ERR_INIT + 0;*/
    resp = diskStop(1);
    if (resp)return resp;

    resp = diskCmd(CMD8, 0x1aa);
    for (i = 0; i < 5; i++)bi_spi(0xff);
    if (resp == 0xff)return DISK_ERR_INIT + 1;
    if (resp != 5)bi_card_type |= SD_V2;

    if (bi_card_type == 2) {

        for (i = 0; i < WAIT_LEN; i++) {


            resp = diskCmd(CMD55, 0xffff);
            if (resp == 0xff)return DISK_ERR_INIT + 2;
            if (resp != 1)continue;

            resp = diskCmd(CMD41, 0x40300000);
            if (resp == 0xff)return DISK_ERR_INIT + 3;
            if (resp != 0)continue;

            break;
        }
        if (i == WAIT_LEN)return DISK_ERR_INIT + 4;

        resp = diskCmd(CMD58, 0);
        if (resp == 0xff)return DISK_ERR_INIT + 5;
        bi_ss_on();
        resp = bi_spi(0xff);
        for (i = 0; i < 3; i++)bi_spi(0xff);
        if ((resp & 0x40))bi_card_type |= 1;

    } else {


        i = 0;

        resp = diskCmd(CMD55, 0);
        if (resp == 0xff)return DISK_ERR_INIT + 6;
        resp = diskCmd(CMD41, 0);
        if (resp == 0xff)return DISK_ERR_INIT + 7;
        cmd = resp;

        for (i = 0; i < WAIT_LEN; i++) {
            if (resp < 1) {

                resp = diskCmd(CMD55, 0);
                if (resp == 0xff)return DISK_ERR_INIT + 8;
                if (resp != 1)continue;

                resp = diskCmd(CMD41, 0);
                if (resp == 0xff)return DISK_ERR_INIT + 9;
                if (resp != 0)continue;

            } else {

                resp = diskCmd(CMD1, 0);
                if (resp != 0)continue;
            }

            break;

        }

        if (i == WAIT_LEN)return DISK_ERR_INIT + 10;

    }

    bi_spi_speed_on();
    disk_addr = 1;
    disk_init_ok = 1;

    return 0;

}

u8 diskOpenRead(u32 saddr) {

    u8 resp;
    if (!(bi_card_type & 1))saddr *= 512;

    resp = diskCmdFast(CMD18, saddr);
    if (resp != 0)return DISK_ERR_RD1;
    bi_ss_on();

    return 0;
}

u8 diskReadToRam(u32 sd_addr, void *dst, u16 slen) {

    u8 resp = 0;

    if (sd_addr != disk_addr) {
        diskCloseRW();
        resp = diskOpenRead(sd_addr);
        if (resp)return resp;
        disk_addr = sd_addr;
    }

    if (resp)return resp;
    disk_addr += slen;

    resp = bi_spi_to_ram(dst, slen);
    if (resp)bi_ss_off();

    return resp;
}

u8 diskReadToRom(u32 sd_addr, void *dst, u16 slen) {

    u8 resp = 0;

    if (sd_addr != disk_addr) {
        diskCloseRW();
        resp = diskOpenRead(sd_addr);
        if (resp)return resp;
        disk_addr = sd_addr;
    }

    if (resp)return resp;
    disk_addr += slen;

    resp = bi_spi_to_rom(dst, slen);
    if (resp)bi_ss_off();

    return resp;

}

void diskCloseRW() {
    if (disk_addr == ~0)return;
    bi_ss_off();
    diskCmdFast(CMD12, 0);
    disk_addr = ~0;
}

u8 diskOpenWrite(u32 saddr) {

    u8 resp;
    if (!(bi_card_type & 1))saddr *= 512;
    resp = diskCmdFast(CMD25, saddr);
    if (resp != 0)return DISK_ERR_WR1;
    bi_ss_on();

    return 0;
}

u8 diskWrite(u32 sd_addr, void *src, u16 slen) {

    u8 resp;
    diskCloseRW();
    resp = diskOpenWrite(sd_addr);
    if (resp != 0)return resp; //DISK_ERR_WR2;

    resp = bi_ram_to_spi(src, slen);

    bi_ss_off();
    if (resp)return resp;
    diskCloseRW();
    return 0;
}

u8 diskStop(u8 forced) {

    u8 i;
    u8 resp;

    if (!forced && !disk_init_ok)return 0;

    i = 0;
    resp = 0;
    while (resp != 1 && i++ < 2) {
        resp = diskCmd(CMD0, 0);
    }

    if (resp != 1)return DISK_ERR_INIT + 0;

    return 0;
}
