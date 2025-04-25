
    
    SPI = 0x7f00
    STATE = 0x7f01
    CFG = 0xfff8
    ST_SPI_BUSY = 7
    
    CFG_SPI_SS = 1
    CFG_SPI_FULL_SPEED = 2

    CMD0 = 0x40
    CMD1 = 0x41
    CMD17 = 0x51

spi_busy:
    ld a, (STATE)
    and a, #1 << ST_SPI_BUSY
    jr nz, spi_busy
    ret

spi_write_a:
    ld (SPI), a
busy:
    ld a, (STATE)
    and a, #1 << ST_SPI_BUSY
    jr nz, busy
    ret

.globl _mmc_cmd_send
_mmc_cmd_send:
    ;SPI_BUSY
    push af

    ld a, (CFG)
    or a, #1 << CFG_SPI_SS
    ld (CFG), a
    
    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)
    
    ld a, (_mmc_cmd)
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, (_mmc_arg + 3)
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, (_mmc_arg + 2)
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, (_mmc_arg + 1)
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, #0x00
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, #0x95
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)

    ld a, (CFG)
    and a, #~(1 << CFG_SPI_SS)
    ld (CFG), a

    pop af
    ret



.globl _mmc_begin_read
_mmc_begin_read:
    push bc
    push hl
    push af


    ld a, (CFG)
    or a, #1 << CFG_SPI_SS
    ld (CFG), a

    ld bc, #0
_wt_cmd17_bg:
    ld a, (SPI)
    cp #0
    jr z, _wt_start_bg
    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;call spi_busy

    dec c
    jr nz, _wt_cmd17_bg
    dec b
    jr nz, _wt_cmd17_bg
_rd_error1_bg:
    ld a, (CFG)
    and a, #~(1 << CFG_SPI_SS)
    ld (CFG), a
    ld a, #1
    ld (_mmc_resp), a
    pop af
    pop hl
    pop bc
    ret

_wt_start_bg:
    ld a, (SPI)
    ld bc, #0
_wt_start2_bg:
    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ld a, (SPI)
    cp #0xfe
    jr z, _start_read_bg
    dec c
    jr nz, _wt_start2_bg
    dec b
    jr nz, _wt_start2_bg
_rd_error2_bg:
    ld a, (CFG)
    and a, #~(1 << CFG_SPI_SS)
    ld (CFG), a
    ld a, #2
    ld (_mmc_resp), a
    pop af
    pop hl
    pop bc
    ret



_start_read_bg:
    pop af
    pop hl
    pop bc
    ret

.globl _mmc_finish_read
_mmc_finish_read:

    push af

    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)
    ld a, #0xff
    call spi_write_a
    ;ld (SPI), a
    ;ld a, (SPI)
    ld a, (CFG)
    and a, #~(1 << CFG_SPI_SS)
    ld (CFG), a
    ld a, #0
    ld (_mmc_resp), a
    pop af
    ret

.globl _mmc_start_read
_mmc_start_read:
    push bc
    push hl
    push af
    ld c, #0
    ld hl, (_mmc_buff_ptr)
_read:
    ;SPI_WR $ff
    ld a, #0xff
    ld (SPI), a
mmc_rd_busy1:
    ld a, (STATE)
    and a, #1 << ST_SPI_BUSY
    jr nz, mmc_rd_busy1

    ld a, (SPI)
    ld (hl), a
    inc hl
    ld a, #0xff
    ld (SPI), a
mmc_rd_busy2:
    ld a, (STATE)
    and a, #1 << ST_SPI_BUSY
    jr nz, mmc_rd_busy2

    ld a, (SPI)
    ld (hl), a
    inc hl
    dec c
    jr nz, _read
    pop af
    pop hl
    pop bc
    ret

.globl _mmc_write_block
_mmc_write_block:
    push bc
    push hl
    push af
    ld a, (CFG)
    or a, #1 << CFG_SPI_SS
    ld (CFG), a

    ld a, #0xff
    call spi_write_a
    ld a, #0xff
    call spi_write_a
    ld a, #0xfe
    call spi_write_a

    ld bc, #512
    ld hl, (_mmc_buff_ptr)
mmc_wr_loop:
    ld a, (hl)
    call spi_write_a
    inc hl

    dec c
    jr nz, mmc_wr_loop
    dec b
    jr nz, mmc_wr_loop

    ld a, #0xff
    call spi_write_a
    ld a, #0xff
    call spi_write_a
    ld a, #0xff
    call spi_write_a

    ld a, (SPI)
    and a, #0x1f
    cp a, #0x05
    jr nz, mmc_wr_err1

    ld bc, #0
mmc_end_loop:
    ld a, #0xff
    call spi_write_a
    ld a, (SPI)
    cp a, #0xff
    jr z, mmc_wr_success
    dec c
    jr nz, mmc_end_loop
    dec b
    jr nz, mmc_end_loop
    jr mmc_wr_err2

mmc_wr_err1:
    ld a, #1
    jr mmc_wr_exit
mmc_wr_err2:
    ld a, #2
    jr mmc_wr_exit
mmc_wr_success:
    ld a, #0
    jr mmc_wr_exit

mmc_wr_exit:
    ld (_mmc_resp), a
    ld a, (CFG)
    and a, #~(1 << CFG_SPI_SS)
    ld (CFG), a
    pop af
    pop hl
    pop bc
    ret
    



.area	_DATA

.globl _mmc_arg
_mmc_arg::
	.ds 4
.globl _mmc_cmd
_mmc_cmd::
	.ds 1
.globl _mmc_resp
_mmc_resp::
	.ds 1
.globl _mmc_buff_ptr
_mmc_buff_ptr::
	.ds 2