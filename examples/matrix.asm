
;
;    Matrix demo for KC 87
;
;    Copyright (c) 2026 René Coignard
;    All rights reserved.
;

include '../source/u880.inc'
include '../source/kc87.inc'

format kcc 'MATRIX'

VIDEO_RAM       := 0EC00h
COLOR_RAM       := 0E800h

ATTR_GREEN      := 20h
ATTR_WHITE      := 70h
CHAR_SPACE      := 20h
TRAIL_LENGTH    := 12

DELAY_TYPE_MIN  := 981
DELAY_ERASE     := 512
DELAY_START     := 17067
DELAY_LINE      := 10240
DELAY_BLACK     := 8533
DELAY_LONG      := 25600
DELAY_FRAME     := 128

org 0300h

start:
    di
    ld sp, 2000h
    call seed
    call clear_screen

    ld de, DELAY_START
    call wait_frames

    ld de, VIDEO_RAM
    ld hl, msg_wake_up_1
    ld b, 9
    call type_line

    ld de, VIDEO_RAM + 4
    ld b, 5
    call erase_chars

    ld de, VIDEO_RAM + 4
    ld hl, msg_wake_up_2
    ld b, 28
    call type_line

    ld de, DELAY_LINE
    call wait_frames
    call clear_screen

    ld de, VIDEO_RAM
    ld hl, msg_matrix_1
    ld b, 10
    call type_line

    ld de, VIDEO_RAM + 4
    ld b, 6
    call erase_chars

    ld de, VIDEO_RAM + 4
    ld hl, msg_matrix_2
    ld b, 16
    call type_line

    ld de, DELAY_LINE
    call wait_frames
    call clear_screen

    ld de, VIDEO_RAM
    ld hl, msg_rabbit_1
    ld b, 32
    call type_line

    ld de, VIDEO_RAM + 11
    ld b, 21
    call erase_chars

    ld de, VIDEO_RAM + 11
    ld hl, msg_rabbit_2
    ld b, 23
    call type_line

    ld de, VIDEO_RAM + 40
    ld hl, msg_rabbit_3
    ld b, 13
    call type_line

    ld de, DELAY_LINE
    call wait_frames
    call clear_screen

    ld de, DELAY_BLACK
    call wait_frames

    ld de, VIDEO_RAM
    ld hl, msg_knock
    ld b, 28
  .knock_loop:
    ld a, (hl)
    ld (de), a
    inc hl
    inc de
    djnz .knock_loop

    ld de, DELAY_LONG
    call wait_frames
    call clear_screen
    ld de, DELAY_BLACK
    call wait_frames

    call init_rain

main_loop:
    ld ix, drop_head
    ld iy, word_pos
    ld c, 0

  .column:
    ld e, c
    ld d, 0
    ld hl, col_idle
    add hl, de
    ld a, (hl)
    or a
    jr z, .col_active

    dec a
    ld (hl), a
    jp nz, .col_next

    call spawn_drop
    jp .col_next

  .col_active:
    ld hl, col_speed
    add hl, de
    ld a, (hl)
    ld hl, col_acc
    add hl, de
    add a, (hl)
    ld (hl), a
    jp nc, .col_next

    ld a, (ix+0)
    ld (head), a
    cp 24
    jr nc, .no_head

    ld a, (head)
    call get_cell_address
    push hl
    call pick_char
    pop hl
    ld (hl), a
    ld de, COLOR_RAM - VIDEO_RAM
    add hl, de
    ld (hl), ATTR_WHITE

  .no_head:
    ld a, (head)
    or a
    jr z, .no_recolor
    cp 25
    jr nc, .no_recolor

    dec a
    call get_cell_address
    ld de, COLOR_RAM - VIDEO_RAM
    add hl, de
    ld (hl), ATTR_GREEN

  .no_recolor:
    ld a, (head)
    sub TRAIL_LENGTH
    jr c, .no_erase
    cp 24
    jr nc, .no_erase

    call get_cell_address
    ld (hl), CHAR_SPACE

  .no_erase:
    ld a, (head)
    inc a
    cp 24 + TRAIL_LENGTH
    jr c, .no_wrap

    call set_col_idle
    jr .col_next

  .no_wrap:
    ld (ix+0), a

  .col_next:
    inc ix
    inc iy
    inc c
    ld a, c
    cp 40
    jp nz, .column

    ld de, DELAY_FRAME
    call wait_frames
    jp main_loop


clear_screen:
    ld hl, VIDEO_RAM
    ld de, VIDEO_RAM + 1
    ld bc, 959
    ld (hl), CHAR_SPACE
    ldir

    ld hl, COLOR_RAM
    ld de, COLOR_RAM + 1
    ld bc, 959
    ld (hl), ATTR_GREEN
    ldir
    ret

type_line:
    ld a, (hl)
    ld (de), a
    inc hl
    inc de
    push hl
    push de
    push bc

    call rand_byte
    ld l, a
    ld h, 0
    add hl, hl
    add hl, hl
    ld de, DELAY_TYPE_MIN
    add hl, de
    ex de, hl
    call wait_frames

    pop bc
    pop de
    pop hl
    djnz type_line
    ret

erase_chars:
    ld l, e
    ld h, d
    ld a, b
    add a, l
    ld l, a
    jr nc, .no_carry
    inc h
  .no_carry:
    dec hl
  .loop:
    ld (hl), CHAR_SPACE
    push hl
    push bc
    ld de, DELAY_ERASE
    call wait_frames
    pop bc
    pop hl
    dec hl
    djnz .loop
    ret

wait_frames:
    ld b, 20
  .inner:
    djnz .inner
    dec de
    ld a, d
    or e
    jr nz, wait_frames
    ret

get_cell_address:
    add a, a
    ld l, a
    ld h, 0
    ld de, row_offsets
    add hl, de
    ld e, (hl)
    inc hl
    ld d, (hl)

    ld hl, VIDEO_RAM
    add hl, de
    ld e, c
    ld d, 0
    add hl, de
    ret

seed:
    ld a, 0E9h
    ld (rand_state+0), a
    ld a, 6Eh
    ld (rand_state+1), a
    ld a, 65h
    ld (rand_state+2), a
    ld a, 52h
    ld (rand_state+3), a
    xor a
    ld (rand_idx), a
    ret

xorshift32:
    ld a, (rand_state+0)
    ld d, a
    ld a, (rand_state+1)
    ld c, a
    ld a, (rand_state+2)
    ld b, a
    ld e, 0

    ld a, 5
  .shift_left_1:
    sla e
    rl d
    rl c
    rl b
    dec a
    jr nz, .shift_left_1

    ld a, (rand_state+0)
    xor e
    ld (rand_state+0), a
    ld a, (rand_state+1)
    xor d
    ld (rand_state+1), a
    ld a, (rand_state+2)
    xor c
    ld (rand_state+2), a
    ld a, (rand_state+3)
    xor b
    ld (rand_state+3), a

    ld a, (rand_state+2)
    ld e, a
    ld a, (rand_state+3)
    ld d, a
    ld b, 0
    ld c, 0

    srl b
    rr c
    rr d
    rr e

    ld a, (rand_state+0)
    xor e
    ld (rand_state+0), a
    ld a, (rand_state+1)
    xor d
    ld (rand_state+1), a
    ld a, (rand_state+2)
    xor c
    ld (rand_state+2), a
    ld a, (rand_state+3)
    xor b
    ld (rand_state+3), a

    ld a, (rand_state+0)
    ld e, a
    ld a, (rand_state+1)
    ld d, a
    ld a, (rand_state+2)
    ld c, a
    ld a, (rand_state+3)
    ld b, a

    ld a, 5
  .shift_left_2:
    sla e
    rl d
    rl c
    rl b
    dec a
    jr nz, .shift_left_2

    ld a, (rand_state+0)
    xor e
    ld (rand_state+0), a
    ld a, (rand_state+1)
    xor d
    ld (rand_state+1), a
    ld a, (rand_state+2)
    xor c
    ld (rand_state+2), a
    ld a, (rand_state+3)
    xor b
    ld (rand_state+3), a
    ret

rand_byte:
    push bc
    push de
    push hl

    ld a, (rand_idx)
    or a
    jr nz, .serve

    call xorshift32
    ld a, 4
    ld (rand_idx), a

  .serve:
    ld a, (rand_idx)
    ld b, a
    ld a, 4
    sub b
    ld e, a
    ld d, 0
    ld hl, rand_state
    add hl, de
    ld c, (hl)

    ld a, b
    dec a
    ld (rand_idx), a
    ld a, c

    pop hl
    pop de
    pop bc
    ret

rand_char:
    call rand_byte
    and 3Fh
    ld e, a
    ld d, 0
    ld hl, charset
    add hl, de
    ld a, (hl)
    ret

pick_char:
    ld a, (iy+0)
    or a
    jp z, rand_char

    dec a
    ld l, a
    ld h, 0
    ld de, fckafd
    add hl, de
    ld a, (hl)
    push af

    ld a, (iy+0)
    cp 6
    jr z, .wrap_word_1
    cp 14
    jr z, .wrap_word_2

    inc a
    ld (iy+0), a
    jr .done

  .wrap_word_1:
    ld a, 1
    ld (iy+0), a
    jr .done

  .wrap_word_2:
    ld a, 7
    ld (iy+0), a

  .done:
    pop af
    ret

set_col_idle:
    call rand_byte
    and 0Fh
    inc a
    ld e, c
    ld d, 0
    ld hl, col_idle
    add hl, de
    ld (hl), a
    ret

spawn_drop:
    call rand_byte
    or a
    jr nz, .normal_drop

    call rand_byte
    and 1
    jr z, .special_drop_1

    ld a, 7
    ld (iy+0), a
    xor a
    ld (ix+0), a
    ret

  .special_drop_1:
    ld a, 1
    ld (iy+0), a
    xor a
    ld (ix+0), a
    ret

  .normal_drop:
    xor a
    ld (iy+0), a
    ld (ix+0), a
    ret

init_rain:
    ld ix, drop_head
    ld iy, word_pos
    ld b, 40
  .loop_pos:
    xor a
    ld (ix+0), a
    ld (iy+0), a
    inc ix
    inc iy
    djnz .loop_pos

    ld ix, col_acc
    ld iy, col_speed
    ld b, 40
  .loop_speed:
    xor a
    ld (ix+0), a
    call rand_byte
    or 80h
    ld (iy+0), a
    inc ix
    inc iy
    djnz .loop_speed

    ld ix, col_idle
    ld b, 40
  .loop_idle:
    call rand_byte
    and 3Fh
    inc a
    ld (ix+0), a
    inc ix
    djnz .loop_idle
    ret

head:  	db 0
rand_idx:      	db 0
rand_state:    	db 0, 0, 0, 0

drop_head:     	rb 40
word_pos:      	rb 40
col_acc:       	rb 40
col_speed:     	rb 40
col_idle:      	rb 40

row_offsets:
    dw 0, 40, 80, 120, 160, 200, 240, 280, 320, 360, 400, 440
    dw 480, 520, 560, 600, 640, 680, 720, 760, 800, 840, 880, 920

fckafd:         db "FCKAFD"
signature:      db "COIGNARD"

charset:        db "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnop0123456789"
		db 21h, 23h, 25h, 26h, 2Ah, 2Bh, 2Dh, 2Fh, 3Ch, 3Eh, 3Fh, 40h

msg_wake_up_1:	db "Wach auf,"
msg_wake_up_2:	db "en Sie auf, Herr Anderson..."
msg_matrix_1:   db "Die Matrix"
msg_matrix_2:   db "Stasi hat Sie..."
msg_rabbit_1:   db "Folgen Sie dem weissen Kaninchen"
msg_rabbit_2:   db "uns zur Klaerung eines "
msg_rabbit_3:   db "Sachverhalts."
msg_knock:      db "Klopf, klopf, Herr Anderson."
