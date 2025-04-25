/* 
 * File:   error.h
 * Author: krik
 *
 * Created on November 13, 2013, 1:03 AM
 */

#ifndef ERROR_H
#define	ERROR_H

#define DISK_ERR_INIT 0xC0
#define DISK_ERR_RD1 0xD2
#define DISK_ERR_RD2 0xD3 //retrun from bios

#define DISK_ERR_WR1 0xD4
#define DISK_ERR_WR2 0xD5
#define DISK_ERR_WR3 0xD6
#define DISK_ERR_WR4 0xD7
#define DISK_ERR_WR5 0xD8


#define FAT_ERR_SMALL_CLUST 0xEF
#define FAT_ERR_NOT_EXIST 0xF0
#define FAT_ERR_EXIST 0xF1
#define FAT_ERR_BAD_NAME 0xF2
#define FAT_ERR_OUT_OF_FILE 0xF3
#define FAT_ERR_NO_FRE_SPACE 0xF4
#define FAT_ERR_NOT_FILE 0xF5
#define FAT_ERR_UNK_TYPE 0xF6

#define FAT_ERR_FILE_MODE1 0xF7
#define FAT_ERR_FILE_MODE2 0xF8
#define FAT_ERR_WR_SIZE 0xF9
#define FAT_ERR_PATH_NOT_EXIST 0xFA
#define FAT_ERR_FAT16 0xFB
#define FAT_ERR_SIZE_EQ 0xFC
#define FAT_ERR_OUT_OF_FILE_WR 0xFD
#define FAT_ERR_NOT_LAST 0xFE
#define FAT_ERR_DIR_MAKE 0xFF


#define ERR_ROM_SIZE 0x30
#define ERR_SAVE_NAME 0x31
#define ERR_GAME_NOT_EXIST 0x32
#define ERR_GAME_NOT_START 0x33
#define ERR_MEM_TYPE 0x34
#define ERR_OUT_OF_RAM1 0x35
#define ERR_OUT_OF_RAM2 0x36
#define ERR_OS_SIZE 0x37
#define ERR_OS_FILE 0x38
#define ERR_OS_NOT_START 0x39

#endif	/* ERROR_H */

