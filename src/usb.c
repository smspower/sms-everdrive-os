
#include "types.h"
#include "everdrive.h"
#include "vdp.h"
#include "prog.h"
#include "ram_app.h"

u32 usb_key;

void usbMode();
void usbWriteGame();
void usbWriteOS();
void startGame();


void usbListener() {


    if (!IS_FIFO_RD_BUSY) {
        usb_key <<= 8;
        usb_key |= FIFO_PORT;
        if (usb_key == 0x5553426D) {
            usbMode();
        }
    }


}

void usbMode() {

    u8 cmd;
    u8 *fifo_port = (u8 *) 0x7f02;
    vdp_clean_screen();
    vdpSetCons(2, 2);
    vdpDrawConsStr("USB mode", PAL1);
    vdp_repaint();

    for (;;) {

        FIFO_RD_BUSY;
        cmd = FIFO_PORT;
        if (cmd != (u8) '+')continue;
        FIFO_RD_BUSY;
        cmd = FIFO_PORT;

        switch (cmd) {

            case 't':
                FIFO_WR_BUSY;
                FIFO_PORT = 'k';
                break;
            case 'g':
                usbWriteGame();
                break;
            case 'o':
                usbWriteOS();
                break;
            case 's':
                FIFO_WR_BUSY;
                FIFO_PORT = OS_VERSION;
                break;
            case 'f':
                FIFO_WR_BUSY;
                *fifo_port = FIRM_VERSION;
                break;
        }
    }
}

void usbInit() {
    usb_key = 0;
}

void usbWriteGame() {


    vdpDrawConsStr("game upload", PAL1);
    vdp_repaint();
    FIFO_RD_BUSY;
    r_write_len = FIFO_PORT;
    if (r_write_len > 16) {
        FIFO_WR_BUSY;
        FIFO_PORT = 'e';
    }
    vdpDrawConsStr("erase...", PAL1);
    vdp_repaint();
    eprErase(0x100000, r_write_len);
    FIFO_WR_BUSY;
    FIFO_PORT = 'k';
    vdpDrawConsStr("write...", PAL1);
    vdp_repaint();
    r_usb_game_load();
    startGame();
}

void usbWriteOS() {

    vdp_off();
    r_usb_os_update();
}
