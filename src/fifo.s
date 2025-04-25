
.equ joy_dat2, 0xa10005
.equ joy_ctr2, 0xa1000b
.equ fifo_clk, 0x40
.equ fifo_oe_c0, 0x00
.equ fifo_oe_c1, 0x40
.equ fifo_we_c0, 0x20
.equ fifo_we_c1, 0x60
.equ fifo_stat, 0x10
.equ fifo_stat_bit, 4

.global fifo_init
.global fifo_rd_ready
.global fifo_wr_ready
.global fifo_byte_rd_safe
.global fifo_byte_rd
.global fifo_byte_wr_safe
.global fifo_byte_wr
.global fifo_to_ram
.global ram_to_fifo
.global vdp_to_fifo
.global fifo_to_vdp

.macro ldp  pidx, reg, ris
    move.l (4 + \pidx * 4 + \ris * 4)(%sp), \reg
.endm


********************************************************************************
fifo_init:
    move.w %d1, -(%sp)
    move.b #0x60, joy_ctr2
    move.b #fifo_oe_c0, joy_dat2
    move.b joy_dat2, %d0
    move.b #fifo_oe_c1, joy_dat2
    move.b #fifo_oe_c0, joy_dat2
    move.b joy_dat2, %d1
    move.b #fifo_oe_c1, joy_dat2
    and.b #fifo_stat, %d0
    and.b #fifo_stat, %d1
    cmp %d0, %d1
    beq 006f
    cmp.b #0, %d1
    bne 000f

    move.b #fifo_oe_c0, joy_dat2
    move.b #fifo_oe_c1, joy_dat2
000:
    move.b #0, %d0
    btst.b #fifo_stat_bit, joy_dat2
    bne 007f
    jsr fifo_byte_rd
    jmp 000b

006:
    move.b #1, %d0
007:
    move.w (%sp)+, %d1
    rts

********************************************************************************
fifo_rd_ready:
    move.b #fifo_oe_c1, joy_dat2
    move.b joy_dat2, %d0
    and.b #fifo_stat, %d0
    rts

********************************************************************************
fifo_wr_ready:
    move.b #fifo_we_c1, joy_dat2
    move.b joy_dat2, %d0
    and.b #fifo_stat, %d0
    move.b #fifo_oe_c1, joy_dat2
    rts
********************************************************************************
fifo_byte_rd_safe:
    move.b #fifo_oe_c1, joy_dat2
000:
    btst.b #4, joy_dat2
    bne 000b
fifo_byte_rd:
    move.w %d1, -(%sp)
    move.b #fifo_oe_c0, joy_dat2
    move.b joy_dat2, %d0
    move.b #fifo_oe_c1, joy_dat2
    lsl.b #4, %d0
    
    move.b #fifo_oe_c0, joy_dat2
    move.b joy_dat2, %d1
    move.b #fifo_oe_c1, joy_dat2
    and.b #0x0F, %d1
    or.b %d1, %d0
    move.w (%sp)+, %d1
    rts

********************************************************************************    
fifo_byte_wr_safe:
    move.b #fifo_we_c1, joy_dat2
000:
    btst.b #4, joy_dat2
    bne 000b
fifo_byte_wr:
    move.w 6(%sp), %d0
    move.w %d1, -(%sp)
    move.b #fifo_we_c1, joy_dat2
    move.b #0x6F, joy_ctr2

    move.b %d0, %d1
    lsr #4, %d1
    or.b #fifo_we_c0, %d1
    move.b %d1, joy_dat2
    eor.b #fifo_clk, %d1
    move.b %d1, joy_dat2

    move.b %d0, %d1
    and.b #15, %d1
    or.b #fifo_we_c1, %d1
    move.b %d1, joy_dat2
    eor.b #fifo_clk, %d1
    move.b %d1, joy_dat2
    eor.b #fifo_clk, %d1
    move.b %d1, joy_dat2

    move.b #0x60, joy_ctr2
    move.b #fifo_oe_c1, joy_dat2

    move.w (%sp)+, %d1
    rts

********************************************************************************
fifo_rd_qq:
    btst.b #4, (%a0)
    bne fifo_rd_qq
    
    move.b %d3, (%a0)
    move.b (%a0), %d0
    move.b %d4, (%a0)
    lsl.b #4, %d0
    
    move.b %d3, (%a0)
    move.b (%a0), %d1
    move.b %d4, (%a0)
    and.b #0x0F, %d1
    or.b %d1, %d0
    rts
********************************************************************************
fifo_wr_qq:
    btst.b #4, (%a0)
    bne fifo_wr_qq
    
    move.b %d0, %d1
    lsr #4, %d1
    or.b %d3, %d1
    move.b %d1, (%a0)
    eor.b #fifo_clk, %d1
    move.b %d1, (%a0)

    move.b %d0, %d1
    and.b #15, %d1
    or.b %d4, %d1
    move.b %d1, (%a0)
    eor.b #fifo_clk, %d1
    move.b %d1, (%a0)
    eor.b #fifo_clk, %d1
    move.b %d1, (%a0)

    move.b %d1, (%a0)
    rts
********************************************************************************
fifo_to_ram:
    movem.l %d0-%d4/%a0-%a2, -(%sp)
    
    move.l #joy_dat2, %a0
    move.b #fifo_oe_c0, %d3
    move.b #fifo_oe_c1, %d4
    ldp 0, %a1, 8
    ldp 1, %d2, 8
    sub.w #1, %d2
000:
    jsr fifo_rd_qq
    move.b %d0, (%a1)+
    dbra %d2, 000b
    
    movem.l (%sp)+, %d0-%d4/%a0-%a2
    rts
********************************************************************************
ram_to_fifo:
    movem.l %d0-%d4/%a0-%a2, -(%sp)
    
    move.l #joy_dat2, %a0
    move.b #fifo_we_c0, %d3
    move.b #fifo_we_c1, %d4
    move.b %d4, (%a0)
    move.b #0x6F, joy_ctr2
    ldp 0, %a1, 8
    ldp 1, %d2, 8
    sub.w #1, %d2
000:
    move.b (%a1)+, %d0
    jsr fifo_wr_qq
    dbra %d2, 000b
    
    move.b #0x60, joy_ctr2
    move.b #fifo_oe_c1, joy_dat2
    movem.l (%sp)+, %d0-%d4/%a0-%a2
    rts


.equ vdp_dat, 0xC00000
.equ vdp_cnt, 0xC00004

********************************************************************************
set_vdp_addres_rd:
    move.l %d0, %d1
    and.w #0x3fff, %d0
    jmp set_vdp_addr
set_vdp_addres_wr:
    move.l %d0, %d1
    and.w #0x3fff, %d0
    or.w #0x4000, %d0
set_vdp_addr:
    move.w #0x8F02, vdp_cnt
    lsr.l #8, %d1
    lsr.l #6, %d1
    move.w %d0, vdp_cnt
    move.w %d1, vdp_cnt+2
    
    rts

********************************************************************************
fifo_to_vdp:
    movem.l %d0-%d4/%a0-%a2, -(%sp)
    
    ldp 0, %d0, 8
    jsr set_vdp_addres_wr
    
    move.l #joy_dat2, %a0
    move.b #fifo_oe_c0, %d3
    move.b #fifo_oe_c1, %d4
    move.l #vdp_dat, %a1
    ldp 1, %d2, 8
    lsr.w #1, %d2
    sub.w #1, %d2
000:

    jsr fifo_rd_qq
    lsl.w #8, %d0
    jsr fifo_rd_qq
    

    move.w %d0, (%a1)
    dbra %d2, 000b
    
    movem.l (%sp)+, %d0-%d4/%a0-%a2
    rts

vdp_to_fifo:
    movem.l %d0-%d4/%a0-%a2, -(%sp)
    
    ldp 0, %d0, 8
    jsr set_vdp_addres_rd
    
    move.l #joy_dat2, %a0
    move.b #fifo_we_c0, %d3
    move.b #fifo_we_c1, %d4
    move.l #vdp_dat, %a1
    move.b %d4, (%a0)
    move.b #0x6F, joy_ctr2
    ldp 1, %d2, 8
    lsr.w #1, %d2
    sub.w #1, %d2
000:
    move.w (%a1), %d0
    jsr fifo_wr_qq
    lsr.w #8, %d0
    jsr fifo_wr_qq

    dbra %d2, 000b
    
    move.b #0x60, joy_ctr2
    move.b #fifo_oe_c1, joy_dat2
    movem.l (%sp)+, %d0-%d4/%a0-%a2
    rts
