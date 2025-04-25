/* 
 * File:   vdp.h
 * Author: KRIK
 *
 * Created on 22 Декабрь 2010 г., 11:42
 */

#ifndef _VDP_H
#define	_VDP_H

#include "types.h"

#define PAL0 0
#define PAL1 (1 << 3)
#define PAL2 1
#define PAL3 (1 << 3 | 1)

void vdp_repaint();
void vdp_init();
void vdp_on();
void vdp_off();
void vdp_clean_screen();
void vdp_draw_str();
void vdp_str_len();
void vdp_wait_vbl();
void vdpDrawStr(u8 *str, u8 x, u8 y, u8 pal);
void vdpDrawConsStr(u8 *str, u8 pal);
void vdpSetCons(u8 x, u8 y);
void vdpDrawStrCenter(u8 *str, u8 y, u8 pal);
void vdpDrawErr(u8 *comment, u8 num);
void vdpDrawNum(u8 *coment, u32 num, u8 x, u8 y, u8 pal);
void vdpDrawStrRt(u8 *str, u8 x, u8 y, u8 pal);
void vdp_load_sms_pal();
void vdp_load_gg_pal();
void vdp_load_sg_pal();

extern u8 vdp_pal;
extern u16 vdp_pos;
extern u16 vdp_tidx_ptr;
extern u16 str_len;
extern u8 vdp_buff[1536];
extern u16 cn_pos;
extern u8 gg_mode;
extern u8 cn_y;



#endif	/* _VDP_H */

