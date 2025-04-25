/* 
 * File:   prog.h
 * Author: KRIK
 *
 * Created on 17 январь 2011 г., 11:26
 */

#ifndef _PROG_H
#define	_PROG_H

#include "types.h"
#include "fat.h"

void eprErase(u32 base_addr, u8 blocks);
u8 progGame(FatRecord *rec);



#endif	/* _PROG_H */

