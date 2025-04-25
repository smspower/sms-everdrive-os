
#include "vdp.h"
#include "joy.h"

u8 vdp_buff[1536];
u16 cn_pos;
u8 str_buff_1[30];
u8 str_buff_2[10];
u8 gg_mode;
u8 cn_y;

u8 STR_intToDecString(u32 val, u8 *str) {

    u8 len = 0;
    u8 ret_len;
    u8 *buff = str_buff_2;


    str += len;
    *buff++ = 0;

    while (val) {

        *buff++ = '0' + val % 10;
        val /= 10;
        len++;
    }
    if (len == 0) {
        *buff++ = '0';
        len++;
    }
    ret_len = len - 1;
    len++;
    while (len--) {
        *str++ = *--buff;
    }

    return ret_len;
}

void vdpDrawNum(u8 *coment, u32 num, u8 x, u8 y, u8 pal) {

    u8 comment_len = 0;
    while (coment[comment_len] != 0) {
        str_buff_1[comment_len] = coment[comment_len];
        comment_len++;
    }
    STR_intToDecString(num, &str_buff_1[comment_len]);
    vdpDrawStr(str_buff_1, x, y, pal);
}

void vdpDrawStr(u8 *str, u8 x, u8 y, u8 pal) {

    vdp_tidx_ptr = (u16) str;
    vdp_pos = (u16) vdp_buff + (x << 1 & 63) + (y << 6);
    vdp_pal = pal;
    vdp_draw_str();

}

void vdpSetCons(u8 x, u8 y) {
    if (gg_mode && x < 6) {
        x = 6;
        y = 3;
    }
    cn_y = y;
    cn_pos = (u16) vdp_buff + (x << 1 & 63) + (y << 6);
}

void vdpDrawConsStr(u8 *str, u8 pal) {

    vdp_tidx_ptr = (u16) str;
    vdp_pos = cn_pos;
    vdp_pal = pal;
    vdp_draw_str();
    cn_pos += 64;
    cn_y++;

}

void vdpDrawStrCenter(u8 *str, u8 y, u8 pal) {

    u8 x;
    vdp_tidx_ptr = (u16) str;
    vdp_str_len();
    x = (32 - str_len) >> 1;

    vdp_pos = (u16) vdp_buff + (x << 1 & 63) + (y << 6);
    vdp_pal = pal;
    vdp_draw_str();

}

void vdpDrawStrRt(u8 *str, u8 x, u8 y, u8 pal) {

    vdp_tidx_ptr = (u16) str;
    vdp_str_len();
    x -= str_len;

    vdp_pos = (u16) vdp_buff + (x << 1 & 63) + (y << 6);
    vdp_pal = pal;
    vdp_draw_str();

}

void vdpDrawErr(u8 *comment, u8 num) {

    num++;
    vdpDrawConsStr(comment, PAL1);
    vdp_repaint();
    joy_wait_read();

}

