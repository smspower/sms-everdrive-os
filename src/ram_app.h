/* 
 * File:   ram_app.h
 * Author: KRIK
 *
 * Created on 22 январь 2011 г., 0:05
 */

#ifndef _RAM_APP_H
#define	_RAM_APP_H

void r_ram_app_init();
void r_erase();
void r_usb_game_load();
void r_usb_os_update();
void r_prog_block_sd();
void r_start_game();
void r_update_os_sd();
extern u8 r_erase_len;
extern u8 r_write_len;
extern u16 r_in_bank_addr;
extern u8 r_start_cfg;



#endif	/* _RAM_APP_H */

