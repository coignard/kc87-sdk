
; MIDI test for KC 87 emulator with MIDI support via PIO

include '../source/u880.inc'
include '../source/kc87.inc'

format kcc 'MIDI'

PORT_MIDI       := 89h
MIDI_NOTE_ON    := 90h
MIDI_NOTE_OFF   := 80h
MIDI_VELOCITY   := 40h
MIDI_PITCH      := 60

org 0300h

start:
    di
    ld sp, 2000h

restart:
    ld ix, delay_table
    ld c, 5

  .bar_loop:
    ld a, (ix+1)
    ld (note_delay), a
    ld a, (ix+2)
    ld (note_delay+1), a
    ld b, (ix+0)

  .note_loop:
    call send_note_on

    ld hl, (note_delay)
    call delay

    call send_note_off

    ld hl, (note_delay)
    call delay

    djnz .note_loop

    ld de, 3
    add ix, de
    dec c
    jr nz, .bar_loop
    jp restart


send_note_on:
    ld a, MIDI_NOTE_ON
    out (PORT_MIDI), a
    ld a, MIDI_PITCH
    out (PORT_MIDI), a
    ld a, MIDI_VELOCITY
    out (PORT_MIDI), a
    ret

send_note_off:
    ld a, MIDI_NOTE_OFF
    out (PORT_MIDI), a
    ld a, MIDI_PITCH
    out (PORT_MIDI), a
    ld a, 0
    out (PORT_MIDI), a
    ret

delay:
    dec hl
    ld a, h
    or l
    jr nz, delay
    ret

note_delay:
    dw 0

delay_table:
    db 2
    dw 47257    ; 1/4

    db 4
    dw 23626    ; 1/8

    db 8
    dw 11811    ; 1/16

    db 16
    dw 5903     ; 1/32

    db 32
    dw 2949     ; 1/64
