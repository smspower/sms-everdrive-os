/* 
 * File:   mmc.h
 * Author: KRIK
 *
 * Created on 22 Декабрь 2010 г., 16:34
 */

#ifndef _MMC_H
#define	_MMC_H

#include "types.h"

u8 mmcInit();
u8 mmcRdBlock(u32 addr, u8 *buff);
void mmcFinishReadInc();
u8 mmcBeginRead();
void mmcSetArg(u32 arg);
u8 mmcWrBlock(u32 addr, u8 *data_ptr);

extern u8 mmc_buff[512];


#endif	/* _MMC_H */

