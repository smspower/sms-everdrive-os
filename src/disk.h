/* 
 * File:   disk.h
 * Author: krik
 *
 * Created on January 10, 2013, 8:48 AM
 */

#ifndef DISK_H
#define	DISK_H

#include "types.h"

#define RAM 1
#define ROM 0

u8 diskInit();
u8 diskOpenRead(u32 saddr);
u8 diskWrite(u32 sd_addr, void *src, u16 slen);
u8 diskStop(u8 forced);

u8 diskReadToRam(u32 sd_addr, void *dst, u16 slen);
u8 diskReadToRom(u32 sd_addr, void *dst, u16 slen);
void diskCloseRW();


#endif	/* DISK_H */

