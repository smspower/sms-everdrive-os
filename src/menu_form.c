
#include "types.h"
#include "vdp.h"
#include "joy.h"
#include "usb.h"
#include "everdrive.h"

u8 menuFormShow(u8 **items, u8 size, u8 step, u8 selector, u8 root) {

    u8 i;
    
    u8 y;
    u8 *ptr = (u8*) 0xfffd;
    vdp_clean_screen();




    for (;;) {

        y = 24 - size * step >> 1;

        for (i = 0; i < size; i++) {

            if (selector == i) {
                vdpDrawStrCenter(items[i], y, PAL1);
            } else {
                vdpDrawStrCenter(items[i], y, PAL0);
            }
            y += step;
        }

        if (root) {
            if (gg_mode) {
                vdpDrawNum("OS v", OS_VERSION, 6, 20, PAL0);
            } else {
                vdpDrawNum("OS v", OS_VERSION, 2, 22, PAL0);
            }

            if (*ptr != 4)vdpDrawStr("reserve OS", 2, 1, PAL1);
        }
/*
        for (c = 0; c < 254; c++) {
            vdp_buff[c << 1] = c;
        }
*/
        vdp_repaint();

        //joy_wait_read();
        for (;;) {
            joy_read();
            if (joy == 0)break;
        }

        for (;;) {
            joy_read();
            if (joy != 0)break;
            usbListener();
        }

        if ((joy & KEY_UP)) {
            selector = selector == 0 ? size - 1 : selector - 1;
            continue;
        }
        if ((joy & KEY_DOWN)) {
            selector = selector == size - 1 ? 0 : selector + 1;
            continue;
        }

        if ((joy & KEY_B)) {
            return selector;
        }

        if ((joy & KEY_C)) {
            return 64 + selector;
        }

    }


}
