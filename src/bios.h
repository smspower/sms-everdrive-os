/* 
 * File:   bios.h
 * Author: krik
 *
 * Created on November 13, 2013, 12:50 AM
 */

#ifndef BIOS_H
#define	BIOS_H

#include "types.h"

void bi_init();
u8 bi_spi(u8 arg);
u8 bi_spi_q(u8 arg);
void bi_spi_qq(u8 arg);
u8 bi_spi_to_ram(void *addr, u16 slen);
u8 bi_spi_to_rom(void *addr, u16 slen);
void bi_ss_on();
void bi_ss_off();
void bi_spi_speed_on();
u8 bi_get_cpld_ver();
u8 bi_ram_to_spi(void *src, u16 slen);

void bi_flash_erase(u32 base_addr, u16 len64k);
void bi_start_game(u16 cfg);
void bi_ram_we_on();
void bi_ram_we_off();

/*
void bi_ram_to_rom(void *src, u32 dst, u16 len4b);
void bi_rom_to_ram(void *src, u32 dst, u16 len4b);
 * */

void bi_usb_to_rom(void *addr, u16 slen);
u8 bi_usb_rd_busy();
u8 bi_usb_wr_busy();
u8 bi_usb_rd_byte();
void bi_usb_wr_byte(u8 dat);
void bi_usb_listener();

u8 bi_is_gg_cart();
//******************************************************************************

#endif	/* BIOS_H */

