
.area	_HEADER (ABS)
    .org 	0
    jp	boot_entry

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

    ;.org	0x100
    .module boot_entry
    JOY_PORT_LO = 0xdc
    JOY_PORT_HI = 0xdd
    KEY_UP = 1 << 0
    KEY_DOWN = 1 << 1
    KEY_LEFT = 1 << 2
    KEY_RIGHT = 1 << 3
    KEY_B = 1 << 4
    KEY_C = 1 << 5

boot_entry:
    di
    ld sp, #0xDFF0
    call _joy_read
    ld a, b
    and #KEY_RIGHT | KEY_C
    cp a, #KEY_RIGHT | KEY_C
    jr z, start_repair
    jr start_os

_joy_read:
    ;push af
    in a, (JOY_PORT_HI)
    xor #0xff
    ld c, a
    in a, (JOY_PORT_LO)
    xor #0xff
    ld b, a
    ;pop af
    ret

start_os:
    ld a, #0
    ld (0xffff), a
    jp start_os_b1 + 32768
start_os_b1:
    ld a, #4
    ld (0xfffd), a
    ld a, #5
    ld (0xfffe), a
    jp 0

start_repair:
    ld a, #0
    ld (0xffff), a
    jp start_repair_b1 + 32768
start_repair_b1:
    ld a, #2
    ld (0xfffd), a
    ld a, #3
    ld (0xfffe), a
    jp 0


    

.area	_DATA



