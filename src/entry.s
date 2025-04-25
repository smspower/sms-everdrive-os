
    .area	_HEADER (ABS)
    .org 	0
    jp	sms_init

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

    JOY_PORT_LO = 0xdc
    JOY_PORT_HI = 0xdd

    .globl	_main
    ;.globl _start_game

  
.org 0x80
.ascii "EDOS"

    .globl _port_cfg
sms_init:
    
    di
    ld sp, #0xDFF0
    ld hl, #0xc002
    ld bc, #8180
    ld a, #0
_cl_ram_loop:
    ld (hl), a
    inc hl
    dec c
    jr nz, _cl_ram_loop
    dec b
    jr nz, _cl_ram_loop

    ld a, (0xc000)
    ld (_port_cfg), a
    call _main
    jp 0

    

.globl _joy_read
_joy_read:
    
    push af
    push bc
    ;in a, (JOY_PORT_HI)
    ;xor #0xff
joy_re:
    ld c, #128
    ld b, a
joy_vf:
    ld a, #0
    in a, (JOY_PORT_LO)
    and a, #0b111111
    cp a, b
    jr nz, joy_re
    dec c
    jr nz, joy_vf
    xor #0xff
    and a, #0b111111
    ld (_joy), a
    pop bc
    pop af
    ret

.globl _joy_wait_read
_joy_wait_read:
    
    push af

joy_wait0:
    call _joy_read
    ld a, (_joy)
    cp a, #0
    jr nz, joy_wait0
joy_wait1:
    call _joy_read
    ld a, (_joy)
    cp a, #0
    jr z, joy_wait1

    pop af
    ret

.area	_DATA
.globl _joy
_joy::
	.ds 1