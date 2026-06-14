
; KRT graphics mode demo for KC 87

include '../source/u880.inc'
include '../source/kc87.inc'
include '../source/compat.inc'

format kcc 'FCKAFD'

PORT_SYS_CTRL   := 088h
PORT_GRAPH_CTRL := 0B8h

COLOR_RAM       := 0E800h
GRAPH_BANK_WIN  := 0EC00h
CELLS_PER_BANK  := 960

org 0300h

start:
    di

    in a, (PORT_SYS_CTRL)
    or 38h
    out (PORT_SYS_CTRL), a

    ld hl, color_data
    ld de, COLOR_RAM
    ld bc, CELLS_PER_BANK
    ldir

    ld hl, pixel_data
    ld a, 08h
  .bank_loop:
    out (PORT_GRAPH_CTRL), a
    ld de, GRAPH_BANK_WIN
    ld bc, CELLS_PER_BANK
    ldir

    inc a
    cp 10h
    jr nz, .bank_loop

  .hold:
    jr .hold

image_data:
    INCBIN 'gfx/fckafd.bin'

color_data := image_data
pixel_data := image_data + CELLS_PER_BANK
