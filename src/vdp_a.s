

    VDP_DAT = 0xbe
    VDP_CNT = 0xbf
    .globl _vdp_buff
    ;.globl _font
    _font = 0xc000 - 1024

.globl _vdp_init
_vdp_init:
vdp_regs_init_:
    push hl
    push bc
    push af
    ld hl, #vdp_reg
    ld b, #vdp_reg_end - vdp_reg
    ld c, #VDP_CNT
    otir

    ld bc, #0
    call set_vram
    ld bc, #0x4000
    ld a, #0x00
vram_clean_:
    
    out (VDP_DAT), a

    dec c
    jr nz, vram_clean_
    dec b
    jr nz, vram_clean_
    
    call vdp_load_font

    pop af
    pop bc
    pop hl
    ret

.globl _vdp_load_sms_pal
_vdp_load_sms_pal:
    push af
    ld hl, #pal
    ld b, #pal_end - pal
    jr load_pal


.globl _vdp_load_gg_pal
_vdp_load_gg_pal:
    push af
    ld hl, #pal_gg
    ld b, #pal_gg_end - pal_gg
    jr load_pal

load_pal:
    
    ld a, #0
    call set_cram
    ld c, #VDP_DAT
    otir
    pop af
    ret
    

    




set_cram:
    out (VDP_CNT), a
    ld a, #0b11000000
    out (VDP_CNT), a
    ret

set_vram:
    ld a, c
    out (VDP_CNT), a
    ld a, b
    and a, #0b00111111
    or a, #0b01000000
    out (VDP_CNT), a
    ret





vdp_load_font:
    ld a, #0
    ld (0xffff), a
    ld hl, #_font
    ld bc, #0x0400
    call set_vram
    ld bc, #1024
tile_loop_:

    ld a, (hl)

    out (VDP_DAT), a
    ld a, #0
    out (VDP_DAT), a
    out (VDP_DAT), a
    out (VDP_DAT), a

    inc hl

    dec c
    jr nz, tile_loop_
    dec b
    jr nz, tile_loop_
load_font_2:
    ld hl, #_font
    ld bc, #0x2400
    call set_vram
    ld bc, #1024
tile_loop2_:

    
    ld a, #0
    out (VDP_DAT), a
    ld a, (hl)
    out (VDP_DAT), a
    ld a, #0
    out (VDP_DAT), a
    out (VDP_DAT), a

    inc hl

    dec c
    jr nz, tile_loop2_
    dec b
    jr nz, tile_loop2_
    
    ret

wait_vbl_f:
    in a, (VDP_CNT)
    rlca
    jr nc, wait_vbl_f
    ret

;ld a, (pal & 1) << 3
;ld (vdp_pal), a
;ld bc, vdp_buff + x * 2 % $40 + y * $40
;call vdp_draw_str

.globl _vdp_off
_vdp_off:
    push af
    ld a, #0b00000000
    out (VDP_CNT), a
    ld a, #0b10000001
    out (VDP_CNT), a
    pop af
    ret

.globl _vdp_on
_vdp_on:
    push af
    ld a, #0b01000000
    out (VDP_CNT), a
    ld a, #0b10000001
    out (VDP_CNT), a
    pop af
    ret

.globl _vdp_draw_str
_vdp_draw_str:
    push af
    push bc
    push hl
    ld bc, (_vdp_pos)
    ld hl, (_vdp_tidx_ptr)

draw_str_loop_:
    ld a, (hl)
    cp #00
    jr z, end_of_loop_
    
    ld (bc), a
    inc bc
    ld a, (_vdp_pal)
    ld (bc), a
    inc bc
    inc hl
    jr draw_str_loop_
end_of_loop_:

    pop hl
    pop bc
    pop af
    ret

.globl _vdp_repaint
_vdp_repaint:
    push hl
    push af
    push bc

    call wait_vbl_f
    ld a, #0
    out (VDP_CNT), a
    ld a,  #0x38 | 0b01000000
    out (VDP_CNT), a
    
    ld b, #0
    ld c, #VDP_DAT
    
    ld hl, #_vdp_buff

    call wait_vbl_f
    otir
    otir
    ;otir
    call wait_vbl_f
    otir
    otir
    call wait_vbl_f
    otir
    otir
    ;otir

    pop bc
    pop af
    pop hl
    ret




.globl _vdp_wait_vbl
_vdp_wait_vbl:
    push af
vbl_wt:
    in a, (VDP_CNT)
    rlca
    jr nc, vbl_wt
    pop af
    ret
    
.globl _vdp_clean_screen
_vdp_clean_screen:
    push bc
    push hl
    push af
    ld bc, #0x3800
    call set_vram
    ld bc, #768
    ld hl, #_vdp_buff
    ld a, #0
scr_clr_loop_:
    ld (hl), a
    inc hl
    ld (hl), a
    inc hl
    dec c
    jr nz, scr_clr_loop_
    dec b
    jr nz, scr_clr_loop_
    pop af
    pop hl
    pop bc
    ret

.globl _vdp_str_len
_vdp_str_len:
    
    push af
    push bc
    push hl
    ld c, #0
    ld hl, (_vdp_tidx_ptr)
str_len_loop_:
    ld a, (hl)
    cp #0
    jr z, str_len_loop_end_
    inc hl
    inc c
    jr str_len_loop_

str_len_loop_end_:
    ld a, c
    ld (_str_len), a

    pop hl
    pop bc
    pop af
    ret

vdp_reg:
    .db 0b00000100		; byte to write to VDP register
    .db 0b10000000		; register to write to (%1000xxxx where xxxx is reg. no.)

    .db 0b10000000
    .db 0b10000001

    .db 0b11111111
    .db 0b10000010

    .db 0b11111111  ; reg 3
    .db 0b10000011
    .db 0b11111111  ; reg 4
    .db 0b10000100

    .db 0b11111111
    .db 0b10000101

    .db 0b11111111
    .db 0b10000110

    .db 0b00000010
    .db 0b10000111

    .db 0b00000000
    .db 0b10001000

    .db 0b00000000
    .db 0b10001001

    .db 0b00000000
    .db 0b10001010
vdp_reg_end:


pal:
    .db 0b00000000
    .db 0b00010101
    .db 0b00000101
    .db 0b00000011
    .db 0b00111100
    .db 0b00110011
    .db 0b00001111
    .db 0b00010110

    .db 0b00011001
    .db 0b00000110
    .db 0b00110101
    .db 0b00100001
    .db 0b00001101
    .db 0b00110111
    .db 0b00100011
    .db 0b00010101

    .db 0b00000000
    .db 0b00111111
    .db 0b00000000
    .db 0b00000011
    .db 0b00111100
    .db 0b00110011
    .db 0b00001111
    .db 0b00010110

    .db 0b00011001
    .db 0b00000110
    .db 0b00110101
    .db 0b00100001
    .db 0b00001101
    .db 0b00110111
    .db 0b00100011
    .db 0b00111111
pal_end:


pal_gg:
    .db 0b00000000
    .db 0b00000000
    .db 0b01100110
    .db 0b00000110
    .db 0b01100110
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000

    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    ;*************
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000

    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    ;*************
    .db 0b00000000
    .db 0b00000000
    .db 0b11101110
    .db 0b00001110
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000

    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    ;*************
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000

    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
    .db 0b00000000
pal_gg_end:


.area	_DATA

.globl _vdp_tidx_ptr
_vdp_tidx_ptr::
	.ds 2
.globl _vdp_pos
_vdp_pos::
	.ds 2

.globl _vdp_pal
_vdp_pal::
	.ds 1

.globl _str_len
_str_len::
	.ds 1