


VDP_DAT = 0xBE
VDP_CNT = 0xBF
JOY_LO = 0xdc
JOY_HI = 0xdd

VDP_VRAM_RD = 0b00000000
VDP_VRAM_WR = 0b01000000
VDP_REG_WR =  0b10000000
VDP_CRAM =    0b11000000



.globl _g_vdp_init
_g_vdp_init:
    ld hl, #vdp_reg
    ld b, #22
    ld c, #VDP_CNT
    otir

    ld a, #0x00
    out (VDP_CNT), a
    ld a, #0x00 | VDP_VRAM_WR
    out (VDP_CNT), a
    
    ld hl, #0x1000
    ld a, #0x00
001$:
    out (VDP_DAT), a
    out (VDP_DAT), a
    out (VDP_DAT), a
    out (VDP_DAT), a
    dec l
    jr nz, 001$
    dec h
    jr nz, 001$
    ret


.globl _g_set_pal
_g_set_pal:
    push ix
    ld	ix,#0
    add	ix,sp

    ld a, #0
    out (VDP_CNT), a
    ld a, 6(ix)
    or a, #VDP_CRAM
    out (VDP_CNT), a

    ld l, 4(ix)
    ld h, 5(ix)
    ld b, 7(ix)
    ld c, #VDP_DAT
    otir

    pop ix
    ret

.globl _g_vram_wr
_g_vram_wr:
    push ix
    ld	ix,#0
    add	ix,sp

    ld a, 6 (ix)
    out (VDP_CNT), a
    ld a, 7 (ix)
    or a, #VDP_VRAM_WR
    out (VDP_CNT), a

    ld l,4 (ix)
    ld h,5 (ix)
    ld c, #VDP_DAT

    ld b,8 (ix)
    ld a,9 (ix)
    inc a
    inc b
    dec b
    jr z, vr1
vr0:
    otir
vr1:
    dec a
    jr nz, vr0

    pop ix
    ret

.globl _g_set_vdp_reg
_g_set_vdp_reg:
    push ix
    ld	ix,#0
    add	ix,sp

    ld a, 5 (ix)
    out (VDP_CNT), a
    ld a, 4 (ix)
    or a, #VDP_REG_WR
    out (VDP_CNT), a

    pop ix
    ret



.globl _sysMemSet
_sysMemSet:
    
    push ix
    ld	ix,#0
    add	ix,sp

    ld l,4 (ix)
    ld h,5 (ix)
    ld a,6 (ix)
    ld b,7 (ix)
    ld c,8 (ix)
    
    inc c
    inc b
    dec b
    jr z, 002$
001$:
    ld (hl), a
    inc hl
    dec b
    jr nz, 001$
002$:
    dec c
    jr nz, 001$

    pop ix
    ret

.globl _sysMemCopy
_sysMemCopy:
    push ix
    ld	ix,#0
    add	ix,sp

    ld l, 4(ix)
    ld h, 5(ix)
    ld e, 6(ix)
    ld d, 7(ix)
    ld c, 8(ix)
    ld b, 9(ix)
    
    ldir

    pop ix
    ret

.globl _gfx_buff
.globl _gRepaint
_gRepaint:
    ld hl, #_gfx_buff
    ld a, #0x00
    out (VDP_CNT), a
    ld a, #0x38 | VDP_VRAM_WR
    out (VDP_CNT), a

    ld b, #0
    ld c, #VDP_DAT
    
    call _gVsync
    otir
    otir
    
    
    call _gVsync
    otir
    otir

    call _gVsync
    otir
    otir


    ret

.globl _gVsync
_gVsync:
    in a, (VDP_CNT)
    rlca
    jr c, _gVsync
1$:
    in a, (VDP_CNT)
    rlca
    jr nc, 1$
    ret

.globl _sys_joy_read
_sys_joy_read:
    call _gVsync
    in a, (JOY_LO)
    xor #0xff
    ld l, a
    
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