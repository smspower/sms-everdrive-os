.area	_HEADER (ABS)
.org 	0
    jp	sms_rst

.org	0x08
    reti
.org	0x10
    reti
.org	0x18
    reti
.org	0x20
    reti
.org	0x28
    reti
.org	0x30
    reti
.org	0x38
    reti
.org	0x66
    reti

.org 0x80
.ascii "EDOS"
.area	_CODE
.globl _os_ver
_os_ver:
.db 9




VDP_DAT = 0xBE
VDP_CNT = 0xBF

VDP_CRAM = 0b11000000

.globl	_main
.globl _cart_3e

sms_rst:
    di
    ld sp, #0xDFF0
    ld a, (0xc000)
    ld (_cart_3e), a
    jp _main

;ram regs. write only
REG_CFG = 0xfff8
REG_KEY = 0xfff9
REG_BNK_CFG0 = 0xfffa
REG_BNK_CFG1 = 0xfffb
REG_BNK_CFG2 = 0xfffc

REG_SPI = 0x7f00
REG_STATE = 0x7f01
REG_FIFO = 0x7f02
REG_FIRM = 0x7f03

ST_SPI_BUSY = 128
ST_ROM_BUSY = 64
ST_FIFO_RD_BUSY = 32
ST_FIFO_WR_BUSY = 16
ST_DEV_GG = 8

CFG_ROM_REGS_ON = 1
CFG_SPI_SS = 2
CFG_SPI_FULL_SPEED = 4
CFG_CDM_MAP = 8
CFG_AUTO_BUSY = 16
CFG_ROM_WE_OFF = 32
CFG_SMS_MODE = 64

BCFG_ROM_BANK = 2
BCFG_RAM_BANK = 4
BCFG_RAM_NROM = 8

.globl _bi_init
.globl _bi_spi_qq
.globl _bi_spi_q
.globl _bi_spi
.globl _bi_ss_on
.globl _bi_ss_off
.globl _bi_spi_speed_on
.globl _bi_spi_to_ram
.globl _bi_flash_erase
.globl _bi_usb_rd_busy
.globl _bi_usb_wr_busy
.globl _bi_usb_rd_byte
.globl _bi_usb_wr_byte
.globl _bi_spi_to_rom
.globl _bi_start_game
.globl _bi_get_cpld_ver
.globl _bi_set_ram_bank
.globl _bi_set_rom_bank
.globl _bi_ram_to_spi
.globl _bi_is_gg_cart
.globl _bi_install_os
.globl _bi_usb_to_rom

.globl _flash_erase_code
.globl _spi_to_rom_code
.globl _usb_to_rom_code

_bi_init:
    ld a, #0x00
    ld (REG_KEY), a
    ld a, #0x52
    ld (REG_KEY), a
    ld a, #0x45
    ld (REG_KEY), a
    ld a, #0x4E
    ld (REG_KEY), a

    ld a, #CFG_ROM_REGS_ON
    ld (REG_CFG), a

    ld a, #BCFG_ROM_BANK
    ld (REG_BNK_CFG2), a
    
    ld hl, #bi_flash_erase_code
    ld de, #_flash_erase_code
    ld bc, #128
    ldir

    ld hl, #bi_spi_to_rom_code
    ld de, #_spi_to_rom_code
    ld bc, #128
    ldir

    ld hl, #bi_usb_to_rom_code
    ld de, #_usb_to_rom_code
    ld bc, #128
    ldir

    ret

_bi_spi_q:
_bi_spi:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld a, 4(ix)
    ld (REG_SPI), a
1$:
    ld a, (REG_STATE)
    rlca
    jr c, 1$
    ld a, (REG_SPI)
    ld l, a

    pop ix
    ret

_bi_spi_qq:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld a, 4(ix)
    ld (REG_SPI), a
1$:
    ;ld a, (REG_STATE)
    ;rlca
    ;jr c, 1$

    pop ix
    ret


_bi_ss_on:
    ld a, (REG_CFG)
    or a, #CFG_SPI_SS
    ld (REG_CFG), a
    ret

_bi_ss_off:
    ld a, (REG_CFG)
    and a, #~CFG_SPI_SS
    ld (REG_CFG), a
    ret

_bi_spi_speed_on:
    ld a, (REG_CFG)
    or a, #CFG_SPI_FULL_SPEED
    ld (REG_CFG), a
    ret

bi_spi_ff:
    ld a, #0xff
bi_spi_a:
    ld (REG_SPI), a
1$:
    ld a, (REG_STATE)
    rlca
    jr c, 1$
    ld a, (REG_SPI)
    ret



bi_spi_to_ram_256:
    ld de, #REG_SPI
1$:
    nop
    ld a, (de)
    ld (hl), a

    ld a, #0xff
    ld (de), a

    inc hl

    dec c
    jr nz, 1$
    ret

_bi_spi_to_ram:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld l, 4(ix)
    ld h, 5(ix)
    ld b, 6(ix)
    ld c, #0  
1$:
    ld de, #0
11$:
    call bi_spi_ff
    cp #0xfe
    jr z, 3$
    dec d
    jr nz, 11$
    dec e
    jr nz, 11$
    jr err_fe
3$:
    ld a, #0xff
    ld (REG_SPI), a

    call bi_spi_to_ram_256
    call bi_spi_to_ram_256
       
    call bi_spi_ff
    ;call bi_spi_ff

    dec b
    jr nz, 1$

    ld l, #0

    pop ix
    ret
err_fe:
    ld a, #0xD3
    ld l, a
    pop ix
    ret





_bi_usb_rd_busy:
    ld a, (REG_STATE)
    and a, #ST_FIFO_RD_BUSY
    ld l, a
    ret

_bi_usb_wr_busy:
    ld a, (REG_STATE)
    and a, #ST_FIFO_WR_BUSY
    ld l, a
    ret

_bi_usb_rd_byte:
    ld a, (REG_STATE)
    and a, #ST_FIFO_RD_BUSY
    jr nz, _bi_usb_rd_byte
    ld a, (REG_FIFO)
    ld l, a
    ret


_bi_usb_wr_byte:
    ld a, (REG_STATE)
    and a, #ST_FIFO_WR_BUSY
    jr nz, _bi_usb_wr_byte
    
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld a, 4(ix)
    ld (REG_FIFO), a
    
    pop ix
    ret

;void bi_flash_erase(u32 base_addr, u16 len64k);
_bi_flash_erase:
    push ix
    ld	ix,#0
    add	ix,sp

    ld a, 6(ix)
    ;ld a, #16
    sla a
    sla a
    ld (0xffff), a
    ld b, 8(ix)

    jp _flash_erase_code
bi_flash_erase_code:
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
1$:
    ld a, #0x30
    ld (0x8000), a
    ld (0xa000), a
    ld a, (0xffff)
    inc a
    ld (0xffff), a
    dec c
    jr nz, 1$
    ld c, #4
    dec b
    jr nz, 1$

2$:;wait ry
    ld a, (0x8000)
    ld b, a
    ld a, (0x8000)
    cp a, b
    jr nz, 2$

    pop ix
    ret

;u8 bi_spi_to_rom(void *addr, u16 slen);
_bi_spi_to_rom:
    push ix
    ld	ix,#0
    add	ix,sp

    ld l, 4(ix)
    ld h, 5(ix)
    ld b, 6(ix)

    ld a, (hl)
    
    srl a
    srl a
    srl a
    srl a
    srl a
    ld c, a
    inc hl
    ld a, (hl)
    
    sla a
    sla a
    sla a
    or a, c
    ;ld a, #0
    ld (0xffff), a
    dec hl
    
    ld a, (hl)
    and a, #31
    sla a
    add a, #0x80
    ld h, a
    ld l, #0

    ld de, #REG_SPI
1$:;copy block 512b
    call _spi_to_rom_code
    call bi_spi_ff
    call bi_spi_ff
    dec b
    jr nz, 1$

    ld a, #0
    ld l, a
    pop ix

    ret


bi_spi_to_rom_code:
;unlock bypass
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x20
    ld (0x8aaa), a
1$:;wait fe
    ld a, #0xff
    ld (de), a
2$:
    ld a, (REG_STATE)
    rlca
    jr c, 2$

;copy 512b
    ld a, (de)
    cp a, #0xfe
    jr nz, 1$
3$:
    ld a, #0xff
    ld (de), a

;may be dangerous if rom busy 
    
4$:
    ld a, (REG_STATE) 
    and a, #ST_ROM_BUSY | ST_SPI_BUSY
    jr nz, 4$
    
    ld a, #0xa0
    ld (hl), a
    
    ld a, (de)
    ld (hl), a
    inc l
    jr nz, 3$
    inc h

    ld a, h
    and a, #1
    jr nz, 3$

5$:;block 512b  end. wait ry
    ld a, (REG_STATE) 
    and a, #ST_ROM_BUSY
    jr nz, 5$

;unlock bypass reset
    ld a, #0x90
    ld (0x8000), a
    ld a, #0x00
    ld (0x8000), a

    ld a, h
    cp a, #0xc0
    jr nz, 6$
;inc block 16k
    ld a, (0xffff)
    inc a
    ld (0xffff), a
    ld hl, #0x8000
6$:
    ret




_bi_get_cpld_ver:
    ld a, (REG_FIRM)
    ld l, a
    ret

_bi_set_ram_bank:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld a, 4(ix)
    and a, #1
    sla a
    sla a
    or a, #BCFG_RAM_NROM
    ld (REG_BNK_CFG2), a

    pop ix
    ret


_bi_set_rom_bank:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld a, 4(ix)
    and a, #1
    sla a
    ld (REG_BNK_CFG2), a

    pop ix
    ret

_bi_ram_to_spi:
    push ix
    ld	ix,#0
    add	ix,sp
    
    ld l, 4(ix)
    ld h, 5(ix)
    ld b, 6(ix)
    ld c, #0

1$:
    call bi_spi_ff
    call bi_spi_ff
    ld a, #0xfc
    call bi_spi_a
2$:
    ld a, (hl)
    call bi_spi_a
    inc hl
    ld a, (hl)
    call bi_spi_a
    inc hl
    dec c
    jr nz, 2$

    call bi_spi_ff
    call bi_spi_ff
    call bi_spi_ff

    and a, #0x1f
    cp a, #0x05
    jr nz, wr_err1

    ld de, #0
3$:
    call bi_spi_ff
    cp a, #0xff
    jr z, 4$
    inc d
    jr nz, 3$
    inc e
    jr nz, 3$
    jr wr_err2

4$:
    dec b
    jr nz, 1$


    ld a, #0xfd
    call bi_spi_a
    ld a, #0xff
    call bi_spi_a
    ld de, #0
5$:
    call bi_spi_ff
    and a, #1
    cp a, #0
    jr nz, 6$
    inc d
    jr nz, 5$
    inc e
    jr nz, 5$
    jr wr_err3

6$:
    
    ld a, #0
    ld l, a
    pop ix
    ret

wr_err1:
    ld a, #0xd5
    jr wr_err
wr_err2:
    ld a, #0xd6
    jr wr_err
wr_err3:
    ld a, #0xd7
wr_err:
    ld l, a
    pop ix
    ret


_bi_is_gg_cart:
    ld a, (REG_STATE)
    and a, #ST_DEV_GG
    ld l, a
    ret


;7	Expansion slot enable	SMS2, GG, Genesis
;6	Cartridge slot enable	GG, Genesis
;5	Card slot enable	SMS2, GG, Genesis
;4	RAM enable	 
;3	BIOS ROM enable	Genesis
;2	I/O enable	GG, Genesis
;1	Unknown	 
;0	Unknown	 


_bi_start_game:
    push ix
    ld	ix,#0
    add	ix,sp
    ld a, (_cart_3e)
    ld (0xc800), a
    ld a, 4(ix)
    ld hl, #start_game_code
    ld de, #0xc000
    ld bc, #128
    ldir
    jp 0xc000
start_game_code:
    ld b, a
    ld a, #0
    ld (0xfffd), a
    ld a, #1
    ld (0xfffe), a
    ld a, #2
    ld (0xffff), a

    ld a, #BCFG_ROM_BANK
    ld (REG_BNK_CFG0), a
    ld (REG_BNK_CFG1), a
    ld (REG_BNK_CFG2), a

    ;ld a, (_r_start_cfg)
    ;ld a, #64 | 32
    ld a, b
    ld (REG_CFG), a
    ld a, #0
    ld (REG_KEY), a
    ld sp, #0xDFF0
    
    ;ld a, (0xc800)
    ld a, #~(128 | 64 | 16 | 4)
    ld (0xc000), a
    ;out (0x3e), a

    ;ld a, #~(1 << 6 | 1<< 4 | 1 << 2)
    ;ld (0xc000), a
    ;ld a, #~(1 << 6 | 1<< 4 | 1 << 2)
    ;out (0x3e), a


    jp 0


_bi_install_os:
    ld hl, #install_os_code
    ld de, #0xc000
    ld bc, #256
    ldir
    jp 0xc000
install_os_code:
    ld a, #0
    ld (REG_BNK_CFG2), a
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
1$:;wait erase
    ld a, (REG_STATE)
    and #ST_ROM_BUSY
    jr nz, 1$


2$:;prog block 16k
    ld hl, #0x8000
3$:;AAA AA 555 55 AAA A0 PA PD
    ld a, (REG_STATE)
    and #ST_ROM_BUSY
    jr nz, 3$
    
    ld a, #BCFG_ROM_BANK
    ld (REG_BNK_CFG2), a
    ld a, (hl)
    push af

    ld a, #0
    ld (REG_BNK_CFG2), a
    
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0xA0
    ld (0x8aaa), a

    pop af
    ld (hl), a

    inc hl
    ld a, h
    cp a, #0xc0
    jr nz, 3$

    ld a, (0xffff)
    inc a
    ld (0xffff), a
    cp a, #6
    jr nz, 2$

4$:
    ld a, (REG_STATE)
    and #ST_ROM_BUSY
    jr nz, 4$

    ld a, #4
    ld (0xfffd), a
    ld a, #5
    ld (0xfffe), a


    ld a, #0
    ld (REG_BNK_CFG0), a
    ld (REG_BNK_CFG1), a
    ld (REG_CFG), a
    ld (REG_KEY), a

    jp 0

;*******************************************************************************
_bi_usb_to_rom:
    push ix
    ld	ix,#0
    add	ix,sp

    ld l, 4(ix)
    ld h, 5(ix)
    ld b, 6(ix)

    ld a, (hl)
    
    srl a
    srl a
    srl a
    srl a
    srl a
    ld c, a
    inc hl
    ld a, (hl)
    
    sla a
    sla a
    sla a
    or a, c
    ;ld a, #0
    ld (0xffff), a
    dec hl
    
    ld a, (hl)
    and a, #31
    sla a
    add a, #0x80
    ld h, a
    ld l, #0

    ld de, #REG_FIFO
1$:;copy block 512b
    call _usb_to_rom_code
    dec b
    jr nz, 1$

    pop ix
    ret


bi_usb_to_rom_code:
;unlock bypass
    ld a, #0xaa
    ld (0x8aaa), a
    ld a, #0x55
    ld (0x8555), a
    ld a, #0x20
    ld (0x8aaa), a
1$:
    
4$:
    ld a, (REG_STATE) 
    and a, #ST_ROM_BUSY | ST_FIFO_RD_BUSY
    jr nz, 4$

    ld a, #0xa0
    ld (hl), a

    ld a, (de)
    ld (hl), a
    inc l
    jr nz, 1$
    inc h

    ld a, h
    and a, #1
    jr nz, 1$

5$:;block 512b  end. wait ry
    ld a, (REG_STATE) 
    and a, #ST_ROM_BUSY
    jr nz, 5$

;unlock bypass reset
    ld a, #0x90
    ld (0x8000), a
    ld a, #0x00
    ld (0x8000), a

    ld a, h
    cp a, #0xc0
    jr nz, 6$
;inc block 16k
    ld a, (0xffff)
    inc a
    ld (0xffff), a
    ld hl, #0x8000
6$:
    ret