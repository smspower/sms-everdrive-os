/* 
 * File:   os.h
 * Author: krik
 *
 * Created on November 13, 2013, 1:02 AM
 */

#ifndef OS_H
#define	OS_H

#include "types.h"
#include "fat.h"

u8 osInit();
void osUsbListener();
u8 osFileMenu(FatFullRecord *rec);
void *osMallocRam(u16 size);
void osReleaseRam(u16 size);
u8 osStartGame();
u8 osMainMenu();
u8 osExitBrowser();
u16 osGetDate();
u16 osGetTime();


#ifdef CART_MSED
#define SYS_SMS
#define FAT_SEEK_STEP 64
#define OS_MAX_DIR_SIZE 512
#endif

#ifdef CART_EDMD
#define SYS_SMD
#define MOTO_ORDER
#define FAT_SEEK_STEP 1
#define OS_MAX_DIR_SIZE 1024
#endif

#ifdef CART_GBX
#define SYS_GBX
#define FAT_SEEK_STEP 64
#define OS_MAX_DIR_SIZE 1024
#endif

#endif	/* OS_H */

