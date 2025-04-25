/* 
 * File:   system.h
 * Author: krik
 *
 * Created on November 13, 2013, 12:14 AM
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#include "types.h"
#include "fat.h"
#include "os.h"

//******************************************************************************sega spec
void gSetPal(u16 pal);
void gSetPlan(u16 plan);
void gCleanPlan(u16 plan);
void sysInitZ80();
//******************************************************************************


void sysInit();
void gSetSGpal();
void sysJoyWait();
u16 sysJoyRead();

void gVsync();
void gSetFont(u8 *ptr);
void gRepaint();

void gCleanScreen();
void gFillRect(u16 data, u8 x, u8 y, u8 w, u8 h);
void gDrawString(u8 *str, u8 x, u8 y);
void gAppendString(u8 *str);
void gConsPrint(u8 *str);
void gDrawString(u8 *str, u8 x, u8 y);
void gSetXY(u8 x, u8 y);
void gDrawStringCx(u8 *str, u8 y);
void gAppendHex8(u8 num);
void gAppendHex16(u16 num);
void gAppendHex16SW(u16 num);
void gAppendHex32(u32 num);
void gAppendNum(u32 num);
void gAppendStringMl(u8 *str, u16 len);
void gDrawStringMl(u8 *str, u8 x, u8 y, u16 len);
void gMoveXY(s8 x, s8 y);
void gAppendChar(u8 str);
void gDrawNum(u32 num, u8 x, u8 y);
void gCopyActiveToRam(u16 *buff);
void gCopyRamToBack(u16 *buff);
void gDrawCursor(u8 *str, u16 x, u16 y);
u8 gScreenW();
u8 gScreenH();

void guiPrintError(u8 code);
u8 guiDrawBrowser(u8 full_repaint, u16 selector);
u8 guiDrawMenu(u8 *str[], u16 def_select);
void guiDrawForm(u16 x, u16 y, u16 w, u16 h);
u8 guiHexView(FatFullRecord *rec);
u8 guiGetMaxRows();

u8 sysGetRomRegion(u8 *rom_hdr);
void sysMemSet(void *dst, u8 val, u16 len);
void sysMemCopy(u8 *src, u8 *dst, u16 len);


extern u16 joy;

#ifdef SYS_SMD
#define JOY_B   0x0040
#define JOY_A   0x0010
#define JOY_SEL 0x0020
#define JOY_STA 0x0080
#define JOY_U   0x0001
#define JOY_D   0x0002
#define JOY_L   0x0004
#define JOY_R   0x0008

#define REGION_J 0x00
#define REGION_MULTI 0x40
#define REGION_U 0x80
#define REGION_E 0xC0
#define REGION_J_PAL 0x40
#endif


#ifdef SYS_SMS
#define JOY_B   0x0010
#define JOY_A   0x0020
#define JOY_SEL 0x00FE
#define JOY_STA 0x00FF
#define JOY_U   0x0001
#define JOY_D   0x0002
#define JOY_L   0x0004
#define JOY_R   0x0008
#endif


#ifdef SYS_GBX
#define JOY_B   0x0002
#define JOY_A   0x0001
#define JOY_SEL 0x0004
#define JOY_STA 0x0008
#define JOY_U   0x0040
#define JOY_D   0x0080
#define JOY_L   0x0020
#define JOY_R   0x0010

typedef struct {
    u8 mapper;
    u8 rom_size;
    u8 ram_size;
    u8 sav_ram;
    u8 swap_keys;
    u8 rom_name[208];
    u16 crc;
} Options;

#endif


#endif	/* SYSTEM_H */

