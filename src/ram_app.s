
    APP_USB_OS = 1
    APP_USB_GAME = 2
    APP_ERASE = 3
    APP_SD_PROG = 4
    APP_START_GAME = 5
    APP_UPDATE_OS_SD = 6
    
    STATE = 0x7f01
    FIFO = 0x7f02
    CFG = 0xfff8
    KEY_REG = 0xfff9
    SPI = 0x7f00

    ST_SPI_BUSY = 7
    ST_ROM_BUSY = 6
    ST_FIFO_RD_BUSY = 5
    ST_FIFO_WR_BUSY = 4

    BANK_CFG0_REG = 0xfffa
    BANK_CFG1_REG = 0xfffb
    BANK_CFG2_REG = 0xfffc

    BCFG_ROM_BANK = 1
    BCFG_RAM_BANK = 2
    BCFG_RAM_NROM = 3
    

.globl _r_ram_app_init
_r_ram_app_init:
    push af
    ld a, #0
    ld (ram_app_idx), a
    pop af
    ret


ram_mudule_init:
    ld a, (ram_app_idx)
    ld b, a
    ld a, (req_app_idx)
    cp a, b
    jr nz, load_ram_modile
    ret
load_ram_modile:
    ld bc, #ram_buff
    ld (ram_app_idx), a
    cp #APP_SD_PROG
    jr z, load_sd_prog
    cp #APP_USB_OS
    jr z, load_usb_os
    cp #APP_USB_GAME
    jr z, load_usb_game
    cp #APP_ERASE
    jr z, load_erase
    cp #APP_START_GAME
    jr z, start_game
    cp #APP_UPDATE_OS_SD
    jr z, update_os_sd
    
    

load_sd_prog:
    ld de, #prog_sd_code
    ld h, #prog_sd_code_end - prog_sd_code
    jr start_ram_load

load_usb_os:
    ld de, #usb_os_code
    ld h, #usb_os_code_end - usb_os_code
    jr start_ram_load
load_usb_game:
    ld de, #usb_game_code
    ld h, #usb_game_code_end - usb_game_code
    jr start_ram_load

load_erase:
    ld de, #erase_code
    ld h, #erase_code_end - erase_code
    jr start_ram_load

start_game:
    ld de, #start_game_code
    ld h, #start_game_code_end - start_game_code
    jr start_ram_load

update_os_sd:
    ld de, #update_os_sd_code
    ld h, #update_os_sd_code_end - update_os_sd_code
    jr start_ram_load


start_ram_load:
    ld a, (de)
    ld (bc), a
    inc de
    inc bc
    dec h
    jr nz, start_ram_load
    ret

begin_ram_app:
    ld (req_app_idx), a
    call ram_mudule_init
    call ram_buff

    pop hl
    pop de
    pop bc
    pop af
    ret

;******************************************ram modules

.globl _r_usb_os_update
_r_usb_os_update:
    ld bc, #16384
    

    ld a, #APP_USB_OS
    jp begin_ram_app

;AAA AA 555 55 AAA 20
usb_os_code:

    ld a, #0
    ld (BANK_CFG2_REG), a
    ld a, #4
    ld (0xffff), a

    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x80
    ld (0x8aaa), a
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x30
    ld (0x8000), a
    ;ld (0x6000), a
usb_os_erase_ry:
    ld a, (STATE)
    and #1 << ST_ROM_BUSY | 1 << ST_FIFO_WR_BUSY
    jr nz, usb_os_erase_ry
    ld a, #"k"
    ld (FIFO), a

    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x20
    ld (0x8aaa), a

usb_os_16k:
    ld bc, #16384
    ld hl, #0x8000
    
usb_os_loop:
    ld a, (STATE)
    and a, #1 << ST_FIFO_RD_BUSY
    jr nz, usb_os_loop

    ld a, #0xa0
    ld (hl), a
    ld a, (FIFO)
    ld (hl), a

usb_os_ry:
    ld a, (STATE)
    and #1 << ST_ROM_BUSY
    jr nz, usb_os_ry

    inc hl
    dec c
    jr nz, usb_os_loop
    dec b
    jr nz, usb_os_loop
    ld a, (0xffff)
    cp a, #4
    jr nz, usb_os_done
    ld a, #5
    ld (0xffff), a
    jr usb_os_16k

usb_os_done:
    ld a, #0x90
    ld (0x8000), a
    ld a, #0x00
    ld (0x8000), a
    ld a, #4
    ld (0xfffd), a
    ld a, #5
    ld (0xfffe), a
    ld a, #0
    ld (BANK_CFG0_REG), a
    ld (BANK_CFG1_REG), a

usb_os_ok:
    ld a, (STATE)
    and #1 << ST_FIFO_WR_BUSY
    jr nz, usb_os_ok
    ld a, #"k"
    ld (FIFO), a
    ld a, #0
    ld (CFG), a
    ld (KEY_REG), a
    jp 0
usb_os_code_end:


.globl _r_usb_game_load
_r_usb_game_load:
    push af
    push bc
    push de
    push hl

    ld a, #0
    ld (0xffff), a
    ld a, #1 << BCFG_ROM_BANK
    ld (BANK_CFG2_REG), a


    ld a, #APP_USB_GAME
    jp begin_ram_app

usb_game_code:

    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x20
    ld (0x8aaa), a
    ld a, (_r_write_len)
    sla a
    ld l, a
    ld h, #2
usb_game_wr64k:
    ld bc, #16384
    ld de, #0x8000
usb_game_loop:
    ld a, (STATE)
    and a, #1 << ST_FIFO_RD_BUSY | 1 << ST_ROM_BUSY
    jr nz, usb_game_loop
    
    ld a, #0xa0
    ld (de), a
    ld a, (FIFO)
    ld (de), a
    inc de

    dec c
    jr nz, usb_game_loop
    dec b
    jr nz, usb_game_loop
    ld a, (0xffff)
    inc a
    ld (0xffff), a

    dec h
    jr nz, usb_game_wr64k
    ld h, #2

usb_game_64k_ok:
    ld a, (STATE)
    and #1 << ST_FIFO_WR_BUSY | 1 << ST_ROM_BUSY
    jr nz, usb_game_64k_ok
    ld a, #"k"
    ld (FIFO), a
    ;ld (FIFO), a

    dec l
    jr nz, usb_game_wr64k



    ld a, #0x90
    ld (0x8000), a
    ld a, #0x00
    ld (0x8000), a
    ret
usb_game_code_end:


;AAA AA 555 55 AAA 80 AAA AA 555 55 BA 30
.globl _r_erase
_r_erase:
    push af
    push bc
    push de
    push hl

    ld a, #APP_ERASE
    jp begin_ram_app

erase_code:
    ld a, (_r_erase_len)
    ld b, a
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x80
    ld (0x8aaa), a
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a

    ld c, #4
erase_loop:
    ld a, #0x30
    ld (0x8000), a
    ld (0xa000), a
    ld a, (0xffff)
    inc a
    ld (0xffff), a
    dec c
    jr nz, erase_loop
    ld c, #4
    dec b
    jr nz, erase_loop

erase_ry:
    ld a, (STATE)
    and #(1 << ST_ROM_BUSY)
    jr nz, erase_ry
    ret

erase_code_end:

.globl _r_prog_block_sd
_r_prog_block_sd:

    push af
    push bc
    push de
    push hl

    ld a, #APP_SD_PROG
    jp begin_ram_app

prog_sd_code:
    push ix
    ;ld a, (CFG)
    ;and a, #~(1 << 4)
    ;ld (CFG), a
    ld hl, (_r_in_bank_addr)
    ld bc, #512

    ld a, #0xff
    ld (SPI), a
prog_sd_loop:

prog_sd_code_ry:
    ld a, (STATE)
    and a, #1 << ST_ROM_BUSY | 1 << ST_SPI_BUSY
    jr nz, prog_sd_code_ry

    ld a, #0xa0
    ld (hl), a
    ld a, (SPI)
    ld (hl), a
    ld a, #0xff
    ld (SPI), a
    inc hl

    dec c
    jr nz, prog_sd_loop
    dec b
    jr nz, prog_sd_loop

prog_sd_code_exit_ry:
    ld a, (STATE)
    and a, #1 << ST_ROM_BUSY
    jr nz, prog_sd_code_exit_ry

    pop ix

    ret
prog_sd_code_end:

    .globl _port_cfg
.globl _r_start_game
_r_start_game:
    ld a, #APP_START_GAME
    jp begin_ram_app
start_game_code:
    ld a, #0
    ld (0xfffd), a
    ld a, #1
    ld (0xfffe), a
    ld a, #2
    ld (0xffff), a

    ld a, #2
    ld (0xfffa), a
    ld (0xfffb), a
    ld (0xfffc), a

    ;ld a, #4
    ;out (0x3e), a

    ld a, (_r_start_cfg)
    ld (0xfff8), a
    ld a, #0
    ld (0xfff9), a
    ld sp, #0xDFF0

    ;ld a, (_port_cfg)
    ld a, #~(1 << 6 | 1<< 4 | 1 << 2)
    ld (0xc000), a
    ld a, #~(1 << 6 | 1<< 4 | 1 << 2)
    out (0x3e), a

    ;ld a, #64
    ;out (0x3e), a

    jp 0
start_game_code_end:


.globl _r_update_os_sd
_r_update_os_sd:
    ld a, #APP_UPDATE_OS_SD
    jp begin_ram_app
update_os_sd_code:
    ld a, #0
    ld (BANK_CFG2_REG), a
    ld a, #4
    ld (0xffff), a
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x80
    ld (0x8aaa), a
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x30
    ld (0x8000), a
    ld (0x8000), a
usb_os_sd_erase_ry:
    ld a, (STATE)
    and #1 << ST_ROM_BUSY
    jr nz, usb_os_sd_erase_ry

    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x20
    ld (0x8aaa), a
    ld d, #60
    ld e, #4
update_os_sd_16k:
    ld hl, #0x8000
    ld bc, #16384
update_os_sd_loop:
   
    ld a, d
    ld (0xffff), a
    ld a, (hl)
    push af
    ld a, e
    ld (0xffff), a

    ld a, #0xa0
    ld (hl), a
    pop af
    ld (hl), a
    inc hl

update_os_sd_ry:
    ld a, (STATE)
    and a, #1 << ST_ROM_BUSY
    jr nz, update_os_sd_ry

    dec c
    jr nz, update_os_sd_loop
    dec b
    jr nz, update_os_sd_loop

    ld a, e
    cp a, #4
    jr nz, sd_os_done
    ld d, #61
    ld e, #5
    jr update_os_sd_16k

sd_os_done:
    ld a, #0x90
    ld (0x8000), a
    ld a, #0x00
    ld (0x8000), a
    ld a, #0
    ld (0xfffd), a
    ld a, #5
    ld (0xfffe), a
    ld a, #0
    ld (BANK_CFG0_REG), a
    ld (BANK_CFG1_REG), a
    ld (CFG), a
    ld (KEY_REG), a
    jp 0
    
update_os_sd_code_end:

.area	_DATA

.globl _r_start_cfg
_r_start_cfg::
        .ds 1
.globl _r_in_bank_addr
_r_in_bank_addr::
	.ds 2
.globl _r_erase_len
_r_erase_len::
	.ds 1
.globl _r_write_len
_r_write_len::
	.ds 1
req_app_idx::
	.ds 1
ram_app_idx::
	.ds 1
ram_buff::
	.ds 256
