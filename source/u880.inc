
;  This file is part of KC 87 SDK.
;
;  Copyright (c) 2026 René Coignard
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Permission is granted to anyone to use this software for any purpose,
;  including commercial applications, and to alter it and redistribute it
;  freely, subject to the following restrictions:
;
;  1. The origin of this software must not be misrepresented; you must not
;     claim that you wrote the original software. If you use this software
;     in a product, an acknowledgment in the product documentation would be
;     appreciated but is not required.
;  2. Altered source versions must be plainly marked as such, and must not be
;     misrepresented as being the original software.
;  3. This notice may not be removed or altered from any source distribution.

element reg8
element B?   : reg8 + 0
element C?   : reg8 + 1
element D?   : reg8 + 2
element E?   : reg8 + 3
element H?   : reg8 + 4
element L?   : reg8 + 5
element A?   : reg8 + 7

element IXH? : reg8 + 4
element IXL? : reg8 + 5
element IYH? : reg8 + 4
element IYL? : reg8 + 5

element reg16
element BC?  : reg16 + 0
element DE?  : reg16 + 1
element HL?  : reg16 + 2
element SP?  : reg16 + 3
element AF?  : reg16 + 4

element AF'?
element IX?
element IY?
element I?
element R?

element condition
element NZ?  : condition + 0
element Z?   : condition + 1
element NC?  : condition + 2
element PO?  : condition + 4
element PE?  : condition + 5
element P?   : condition + 6
element M?   : condition + 7

NV? = PO?
V?  = PE?

calminstruction check_8bit_offset? off*
    check definite off
    jno ok
    check off >= -128 & off <= 127
    jyes ok
    err '8-bit offset out of bounds'
ok:
end calminstruction

iterate <name, opcode>,\
        NOP,  00h, HALT, 76h, DI,   0F3h, EI,   0FBh,\
        EXX,  0D9h,CCF,  3Fh, SCF,  37h,  CPL,  2Fh,\
        RLCA, 07h, RRCA, 0Fh, RLA,  17h,  RRA,  1Fh
    calminstruction name?
        emit 1, opcode
    end calminstruction
end iterate

iterate <name, opcode>,\
        NEG, 44h, RETI, 4Dh, RETN, 45h, RRD, 67h, RLD, 6Fh
    calminstruction name?
        emit 1, 0EDh
        emit 1, opcode
    end calminstruction
end iterate

calminstruction DAA?
    emit 1, 27h
end calminstruction

calminstruction IM? mode*
    check mode = 0
    jyes m0
    check mode = 1
    jyes m1
    check mode = 2
    jyes m2
    err 'invalid interrupt mode'
    exit
m0: emit 1, 0EDh
    emit 1, 46h
    exit
m1: emit 1, 0EDh
    emit 1, 56h
    exit
m2: emit 1, 0EDh
    emit 1, 5Eh
end calminstruction

iterate <name, opcode, op_imm>,\
        ADD, 10000000b, 0xC6,\
        ADC, 10001000b, 0xCE,\
        SUB, 10010000b, 0xD6,\
        SBC, 10011000b, 0xDE,\
        AND, 10100000b, 0xE6,\
        XOR, 10101000b, 0xEE,\
        OR,  10110000b, 0xF6,\
        CP,  10111000b, 0xFE
    calminstruction name? args&
        local a1, a2, op
        match a1=,a2, args
        jyes two_args
        arrange a2, args
        jump parse_src
    two_args:
        check a1 eq a1 element 1
        jno err_arg
        check a1 eq A
        jyes parse_src
        check a1 eq HL
        jyes hl_arith
        check a1 eq IX
        jyes ix_arith
        check a1 eq IY
        jyes iy_arith
        jump err_arg
    parse_src:
        check a2 eq a2 element 1 & a2 metadata 1 relativeto reg8
        jyes r8
        match (op), a2
        jyes mem
        emit 1, op_imm
        emit 1, a2
        exit
    r8:
        compute op, a2 metadata 1 - reg8
        check a2 eq IXH | a2 eq IXL
        jyes r8_ix
        check a2 eq IYH | a2 eq IYL
        jyes r8_iy
        emit 1, opcode or op
        exit
    r8_ix:
        emit 1, 0xDD
        emit 1, opcode or op
        exit
    r8_iy:
        emit 1, 0xFD
        emit 1, opcode or op
        exit
    mem:
        check op eq HL
        jyes m_hl
        check op relativeto IX
        jyes m_ix
        check op relativeto IY
        jyes m_iy
        jump err_arg
    m_hl:
        emit 1, opcode or 6
        exit
    m_ix:
        emit 1, 0xDD
        emit 1, opcode or 6
        compute op, op - IX
        call check_8bit_offset?, op
        emit 1, op
        exit
    m_iy:
        emit 1, 0xFD
        emit 1, opcode or 6
        compute op, op - IY
        call check_8bit_offset?, op
        emit 1, op
        exit
    hl_arith:
        check a2 eq a2 element 1 & a2 metadata 1 relativeto reg16
        jno err_arg
        compute op, a2 metadata 1 - reg16
        check op <= 3
        jno err_arg
        check opcode = 10000000b
        jyes hl_add
        check opcode = 10001000b
        jyes hl_adc
        check opcode = 10011000b
        jyes hl_sbc
        jump err_arg
    hl_add:
        emit 1, 00001001b or (op shl 4)
        exit
    hl_adc:
        emit 1, 0xED
        emit 1, 01001010b or (op shl 4)
        exit
    hl_sbc:
        emit 1, 0xED
        emit 1, 01000010b or (op shl 4)
        exit
    ix_arith:
        check opcode = 10000000b
        jno err_arg
        check a2 eq BC | a2 eq DE | a2 eq IX | a2 eq SP
        jno err_arg
        compute op, 00001001b
        check a2 eq BC
        jyes ix_em
        compute op, 00011001b
        check a2 eq DE
        jyes ix_em
        compute op, 00101001b
        check a2 eq IX
        jyes ix_em
        compute op, 00111001b
    ix_em:
        emit 1, 0xDD
        emit 1, op
        exit
    iy_arith:
        check opcode = 10000000b
        jno err_arg
        check a2 eq BC | a2 eq DE | a2 eq IY | a2 eq SP
        jno err_arg
        compute op, 00001001b
        check a2 eq BC
        jyes iy_em
        compute op, 00011001b
        check a2 eq DE
        jyes iy_em
        compute op, 00101001b
        check a2 eq IY
        jyes iy_em
        compute op, 00111001b
    iy_em:
        emit 1, 0xFD
        emit 1, op
        exit
    err_arg:
        err 'invalid argument'
    end calminstruction
end iterate

iterate <name, op8, op16>,\
        INC, 00000100b, 00000011b,\
        DEC, 00000101b, 00001011b
    calminstruction name? arg*
        local op, a
        check arg eq arg element 1
        jyes reg
        match (a), arg
        jyes mem
        jump err_arg
    reg:
        check arg metadata 1 relativeto reg8
        jyes r8
        check arg metadata 1 relativeto reg16
        jyes r16
        check arg eq IX
        jyes r_ix
        check arg eq IY
        jyes r_iy
        jump err_arg
    r8:
        compute op, arg metadata 1 - reg8
        check arg eq IXH | arg eq IXL
        jyes r8_ix
        check arg eq IYH | arg eq IYL
        jyes r8_iy
        emit 1, op8 or (op shl 3)
        exit
    r8_ix:
        emit 1, 0xDD
        emit 1, op8 or (op shl 3)
        exit
    r8_iy:
        emit 1, 0xFD
        emit 1, op8 or (op shl 3)
        exit
    r16:
        compute op, arg metadata 1 - reg16
        check op <= 3
        jno err_arg
        emit 1, op16 or (op shl 4)
        exit
    r_ix:
        emit 1, 0xDD
        emit 1, op16 or 0x20
        exit
    r_iy:
        emit 1, 0xFD
        emit 1, op16 or 0x20
        exit
    mem:
        check a eq HL
        jyes m_hl
        check a relativeto IX
        jyes m_ix
        check a relativeto IY
        jyes m_iy
        jump err_arg
    m_hl:
        emit 1, op8 or 0x30
        exit
    m_ix:
        emit 1, 0xDD
        emit 1, op8 or 0x30
        compute a, a - IX
        call check_8bit_offset?, a
        emit 1, a
        exit
    m_iy:
        emit 1, 0xFD
        emit 1, op8 or 0x30
        compute a, a - IY
        call check_8bit_offset?, a
        emit 1, a
        exit
    err_arg:
        err 'invalid argument'
    end calminstruction
end iterate

calminstruction LD? args&
    local dst, src, d, s, d_val, s_val
    match dst=,src, args
    jno err_arg
    match (d), dst
    jyes dst_mem
    check dst eq dst element 1
    jno err_arg
    check dst metadata 1 relativeto reg8
    jyes dst_r8
    check dst metadata 1 relativeto reg16
    jyes dst_r16
    check dst eq IX | dst eq IY | dst eq I | dst eq R
    jyes dst_spc
    jump err_arg
dst_mem:
    check d eq HL
    jyes m_hl
    check d relativeto IX
    jyes m_ix
    check d relativeto IY
    jyes m_iy
    check d eq BC
    jyes m_bc
    check d eq DE
    jyes m_de
    check src eq A
    jyes mnn_a
    check src eq src element 1 & src metadata 1 relativeto reg16
    jyes mnn_r16
    check src eq IX
    jyes mnn_ix
    check src eq IY
    jyes mnn_iy
    jump err_arg
m_bc:
    check src eq A
    jno err_arg
    emit 1, 02h
    exit
m_de:
    check src eq A
    jno err_arg
    emit 1, 12h
    exit
m_hl:
    check src eq src element 1 & src metadata 1 relativeto reg8
    jyes mhl_r8
    emit 1, 36h
    emit 1, src
    exit
mhl_r8:
    check src eq IXH | src eq IXL | src eq IYH | src eq IYL
    jyes err_arg
    compute s_val, src metadata 1 - reg8
    emit 1, 01110000b or s_val
    exit
m_ix:
    check src eq src element 1 & src metadata 1 relativeto reg8
    jyes mix_r8
    emit 1, 0xDD
    emit 1, 36h
    compute d, d - IX
    call check_8bit_offset?, d
    emit 1, d
    emit 1, src
    exit
mix_r8:
    check src eq IXH | src eq IXL | src eq IYH | src eq IYL
    jyes err_arg
    compute s_val, src metadata 1 - reg8
    emit 1, 0xDD
    emit 1, 01110000b or s_val
    compute d, d - IX
    call check_8bit_offset?, d
    emit 1, d
    exit
m_iy:
    check src eq src element 1 & src metadata 1 relativeto reg8
    jyes miy_r8
    emit 1, 0xFD
    emit 1, 36h
    compute d, d - IY
    call check_8bit_offset?, d
    emit 1, d
    emit 1, src
    exit
miy_r8:
    check src eq IXH | src eq IXL | src eq IYH | src eq IYL
    jyes err_arg
    compute s_val, src metadata 1 - reg8
    emit 1, 0xFD
    emit 1, 01110000b or s_val
    compute d, d - IY
    call check_8bit_offset?, d
    emit 1, d
    exit
mnn_a:
    emit 1, 32h
    emit 2, d
    exit
mnn_r16:
    compute s_val, src metadata 1 - reg16
    check s_val <= 3
    jno err_arg
    check src eq HL
    jyes mnn_hl
    emit 1, 0xED
    emit 1, 01000011b or (s_val shl 4)
    emit 2, d
    exit
mnn_hl:
    emit 1, 22h
    emit 2, d
    exit
mnn_ix:
    emit 1, 0xDD
    emit 1, 22h
    emit 2, d
    exit
mnn_iy:
    emit 1, 0xFD
    emit 1, 22h
    emit 2, d
    exit
dst_r8:
    compute d_val, dst metadata 1 - reg8
    check src eq src element 1 & src metadata 1 relativeto reg8
    jyes r8_r8
    match (s), src
    jyes r8_mem
    check dst eq A & src eq I
    jyes a_i
    check dst eq A & src eq R
    jyes a_r
    check dst eq IXH | dst eq IXL
    jyes r8n_ix
    check dst eq IYH | dst eq IYL
    jyes r8n_iy
    emit 1, 00000110b or (d_val shl 3)
    emit 1, src
    exit
r8n_ix:
    emit 1, 0xDD
    emit 1, 00000110b or (d_val shl 3)
    emit 1, src
    exit
r8n_iy:
    emit 1, 0xFD
    emit 1, 00000110b or (d_val shl 3)
    emit 1, src
    exit
r8_r8:
    compute s_val, src metadata 1 - reg8
    check dst eq IXH | dst eq IXL
    jyes dst_ix
    check dst eq IYH | dst eq IYL
    jyes dst_iy
    check src eq IXH | src eq IXL
    jyes src_ix
    check src eq IYH | src eq IYL
    jyes src_iy
    emit 1, 01000000b or (d_val shl 3) or s_val
    exit
dst_ix:
    check src eq IYH | src eq IYL | src eq H | src eq L
    jyes err_arg
    emit 1, 0xDD
    emit 1, 01000000b or (d_val shl 3) or s_val
    exit
dst_iy:
    check src eq IXH | src eq IXL | src eq H | src eq L
    jyes err_arg
    emit 1, 0xFD
    emit 1, 01000000b or (d_val shl 3) or s_val
    exit
src_ix:
    check dst eq H | dst eq L
    jyes err_arg
    emit 1, 0xDD
    emit 1, 01000000b or (d_val shl 3) or s_val
    exit
src_iy:
    check dst eq H | dst eq L
    jyes err_arg
    emit 1, 0xFD
    emit 1, 01000000b or (d_val shl 3) or s_val
    exit
r8_mem:
    check s eq HL
    jyes r8_mhl
    check s relativeto IX
    jyes r8_mix
    check s relativeto IY
    jyes r8_miy
    check dst eq A
    jno err_arg
    check s eq BC
    jyes r8_mbc
    check s eq DE
    jyes r8_mde
    emit 1, 3Ah
    emit 2, s
    exit
r8_mbc:
    emit 1, 0Ah
    exit
r8_mde:
    emit 1, 1Ah
    exit
r8_mhl:
    check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
    jyes err_arg
    emit 1, 01000110b or (d_val shl 3)
    exit
r8_mix:
    check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
    jyes err_arg
    emit 1, 0xDD
    emit 1, 01000110b or (d_val shl 3)
    compute s, s - IX
    call check_8bit_offset?, s
    emit 1, s
    exit
r8_miy:
    check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
    jyes err_arg
    emit 1, 0xFD
    emit 1, 01000110b or (d_val shl 3)
    compute s, s - IY
    call check_8bit_offset?, s
    emit 1, s
    exit
a_i:
    emit 1, 0xED
    emit 1, 57h
    exit
a_r:
    emit 1, 0xED
    emit 1, 5Fh
    exit
dst_r16:
    compute d_val, dst metadata 1 - reg16
    check d_val <= 3
    jno err_arg
    match (s), src
    jyes r16_mem
    check dst eq SP & src eq HL
    jyes sp_hl
    check dst eq SP & src eq IX
    jyes sp_ix
    check dst eq SP & src eq IY
    jyes sp_iy
    emit 1, 00000001b or (d_val shl 4)
    emit 2, src
    exit
r16_mem:
    check dst eq HL
    jyes r16m_hl
    emit 1, 0xED
    emit 1, 01001011b or (d_val shl 4)
    emit 2, s
    exit
r16m_hl:
    emit 1, 2Ah
    emit 2, s
    exit
sp_hl:
    emit 1, 0xF9
    exit
sp_ix:
    emit 1, 0xDD
    emit 1, 0xF9
    exit
sp_iy:
    emit 1, 0xFD
    emit 1, 0xF9
    exit
dst_spc:
    check dst eq I & src eq A
    jyes i_a
    check dst eq R & src eq A
    jyes r_a
    match (s), src
    jyes ixiy_mem
    check dst eq IX
    jyes ix_nn
    emit 1, 0xFD
    emit 1, 21h
    emit 2, src
    exit
ix_nn:
    emit 1, 0xDD
    emit 1, 21h
    emit 2, src
    exit
ixiy_mem:
    check dst eq IX
    jyes ix_m
    emit 1, 0xFD
    emit 1, 2Ah
    emit 2, s
    exit
ix_m:
    emit 1, 0xDD
    emit 1, 2Ah
    emit 2, s
    exit
i_a:
    emit 1, 0xED
    emit 1, 47h
    exit
r_a:
    emit 1, 0xED
    emit 1, 4Fh
    exit
err_arg:
    err 'invalid argument'
end calminstruction

iterate <name, op>,\
        PUSH, 11000101b,\
        POP,  11000001b
    calminstruction name? arg*
        local v
        check arg eq arg element 1
        jno err_arg
        check arg metadata 1 relativeto reg16
        jyes r16
        check arg eq IX
        jyes ix
        check arg eq IY
        jyes iy
        jump err_arg
    r16:
        compute v, arg metadata 1 - reg16
        check v = 0 | v = 1 | v = 2 | v = 4
        jno err_arg
        check v = 4
        jno r16_em
        compute v, 3
    r16_em:
        emit 1, op or (v shl 4)
        exit
    ix:
        emit 1, 0xDD
        emit 1, op or 0x20
        exit
    iy:
        emit 1, 0xFD
        emit 1, op or 0x20
        exit
    err_arg:
        err 'invalid argument'
    end calminstruction
end iterate

calminstruction EX? args&
    local a1, a2, p
    match a1=,a2, args
    jno err_arg
    check a1 eq DE & a2 eq HL
    jyes de_hl
    check a1 eq AF & a2 eq AF'
    jyes af_af
    match (p), a1
    jno err_arg
    check p eq SP
    jno err_arg
    check a2 eq HL
    jyes sp_hl
    check a2 eq IX
    jyes sp_ix
    check a2 eq IY
    jyes sp_iy
    jump err_arg
de_hl:
    emit 1, 0xEB
    exit
af_af:
    emit 1, 0x08
    exit
sp_hl:
    emit 1, 0xE3
    exit
sp_ix:
    emit 1, 0xDD
    emit 1, 0xE3
    exit
sp_iy:
    emit 1, 0xFD
    emit 1, 0xE3
    exit
err_arg:
    err 'invalid argument'
end calminstruction

calminstruction JP? args&
    local a1, a2, v
    match a1=,a2, args
    jyes cond
    match (v), args
    jyes ind
    check args eq HL | args eq IX | args eq IY
    jyes ind_no_parens
    emit 1, 0xC3
    emit 2, args
    exit
cond:
    check a1 eq C
    jyes c_cond
    check a1 eq a1 element 1 & a1 metadata 1 relativeto condition
    jno err_arg
    compute v, a1 metadata 1 - condition
    jump cond_ok
c_cond:
    compute v, 3
cond_ok:
    emit 1, 11000010b or (v shl 3)
    emit 2, a2
    exit
ind_no_parens:
    compute v, args
ind:
    check v eq HL
    jyes i_hl
    check v eq IX
    jyes i_ix
    check v eq IY
    jyes i_iy
    jump err_arg
i_hl:
    emit 1, 0xE9
    exit
i_ix:
    emit 1, 0xDD
    emit 1, 0xE9
    exit
i_iy:
    emit 1, 0xFD
    emit 1, 0xE9
    exit
err_arg:
    err 'invalid argument'
end calminstruction

calminstruction JR? args&
    local a1, a2, v, off, origin
    compute origin, $
    match a1=,a2, args
    jyes cond
    emit 1, 0x18
    compute off, args - (origin + 2)
    call check_8bit_offset?, off
    emit 1, off
    exit
cond:
    check a1 eq C
    jyes c_cond
    check a1 eq a1 element 1 & a1 metadata 1 relativeto condition
    jno err_arg
    compute v, a1 metadata 1 - condition
    jump cond_ok
c_cond:
    compute v, 3
cond_ok:
    check v <= 3
    jno err_arg
    emit 1, 00100000b or (v shl 3)
    compute off, a2 - (origin + 2)
    call check_8bit_offset?, off
    emit 1, off
    exit
err_arg:
    err 'invalid condition'
end calminstruction

calminstruction DJNZ? arg*
    local off, origin
    compute origin, $
    emit 1, 0x10
    compute off, arg - (origin + 2)
    call check_8bit_offset?, off
    emit 1, off
    exit
end calminstruction

calminstruction CALL? args&
    local a1, a2, v
    match a1=,a2, args
    jyes cond
    emit 1, 0xCD
    emit 2, args
    exit
cond:
    check a1 eq C
    jyes c_cond
    check a1 eq a1 element 1 & a1 metadata 1 relativeto condition
    jno err_arg
    compute v, a1 metadata 1 - condition
    jump cond_ok
c_cond:
    compute v, 3
cond_ok:
    emit 1, 11000100b or (v shl 3)
    emit 2, a2
    exit
err_arg:
    err 'invalid argument'
end calminstruction

calminstruction RET? args&
    local v
    match , args
    jyes uncond
    check args eq C
    jyes c_cond
    check args eq args element 1 & args metadata 1 relativeto condition
    jno err_arg
    compute v, args metadata 1 - condition
    jump cond_ok
c_cond:
    compute v, 3
cond_ok:
    emit 1, 11000000b or (v shl 3)
    exit
uncond:
    emit 1, 0xC9
    exit
err_arg:
    err 'invalid condition'
end calminstruction

calminstruction RST? arg*
    check arg = 00h | arg = 08h | arg = 10h | arg = 18h | arg = 20h | arg = 28h | arg = 30h | arg = 38h
    jno err_arg
    emit 1, 11000111b or arg
    exit
err_arg:
    err 'invalid restart address'
end calminstruction

iterate <name, opcode>,\
        RLC, 00h, RRC, 08h, RL, 10h, RR, 18h,\
        SLA, 20h, SRA, 28h, SLL,30h, SRL,38h
    calminstruction name? args&
        local v, a, d, dst
        match (a)=,dst, args
        jyes m_dst
        match (a), args
        jyes m_only
        check args eq args element 1 & args metadata 1 relativeto reg8
        jyes r8
        jump err_arg
    m_dst:
        check dst eq dst element 1 & dst metadata 1 relativeto reg8
        jno err_arg
        check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
        jyes err_arg
        compute d, dst metadata 1 - reg8
        jump mem
    m_only:
        compute d, 6
    mem:
        check a eq HL
        jyes m_hl
        check a relativeto IX
        jyes m_ix
        check a relativeto IY
        jyes m_iy
        jump err_arg
    m_hl:
        check d = 6
        jno err_arg
        emit 1, 0xCB
        emit 1, opcode or 6
        exit
    m_ix:
        emit 1, 0xDD
        emit 1, 0xCB
        compute a, a - IX
        call check_8bit_offset?, a
        emit 1, a
        emit 1, opcode or d
        exit
    m_iy:
        emit 1, 0xFD
        emit 1, 0xCB
        compute a, a - IY
        call check_8bit_offset?, a
        emit 1, a
        emit 1, opcode or d
        exit
    r8:
        check args eq IXH | args eq IXL | args eq IYH | args eq IYL
        jyes err_arg
        compute v, args metadata 1 - reg8
        emit 1, 0xCB
        emit 1, opcode or v
        exit
    err_arg:
        err 'invalid argument'
    end calminstruction
end iterate

iterate <name, opbase>,\
        BIT, 01000000b,\
        RES, 10000000b,\
        SET, 11000000b
    calminstruction name? args&
        local bit, src, a, v, d, dst
        match bit=,(a)=,dst, args
        jyes m_dst
        match bit=,src, args
        jyes m_only
        jump err_arg
    m_dst:
        check dst eq dst element 1 & dst metadata 1 relativeto reg8
        jno err_arg
        check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
        jyes err_arg
        compute d, dst metadata 1 - reg8
        jump parse_bit
    m_only:
        check src eq src element 1 & src metadata 1 relativeto reg8
        jyes r8
        match (a), src
        jyes m_only_mem
        jump err_arg
    m_only_mem:
        compute d, 6
    parse_bit:
        check bit >= 0 & bit <= 7
        jno err_arg
    mem:
        check a eq HL
        jyes m_hl
        check a relativeto IX
        jyes m_ix
        check a relativeto IY
        jyes m_iy
        jump err_arg
    m_hl:
        check d = 6
        jno err_arg
        emit 1, 0xCB
        emit 1, opbase or (bit shl 3) or 6
        exit
    m_ix:
        emit 1, 0xDD
        emit 1, 0xCB
        compute a, a - IX
        call check_8bit_offset?, a
        emit 1, a
        emit 1, opbase or (bit shl 3) or d
        exit
    m_iy:
        emit 1, 0xFD
        emit 1, 0xCB
        compute a, a - IY
        call check_8bit_offset?, a
        emit 1, a
        emit 1, opbase or (bit shl 3) or d
        exit
    r8:
        check src eq IXH | src eq IXL | src eq IYH | src eq IYL
        jyes err_arg
        check bit >= 0 & bit <= 7
        jno err_arg
        compute v, src metadata 1 - reg8
        emit 1, 0xCB
        emit 1, opbase or (bit shl 3) or v
        exit
    err_arg:
        err 'invalid argument'
    end calminstruction
end iterate

calminstruction IN? args&
    local dst, src, s, v
    match dst=,src, args
    jyes two_args
    match (s), args
    jno err_arg
    check s eq C
    jno err_arg
    emit 1, 0xED
    emit 1, 70h
    exit
two_args:
    match (s), src
    jno err_arg
    check dst eq A
    jyes a_n
    check s eq C
    jyes r_c
    jump err_arg
a_n:
    check s eq C
    jyes a_c
    emit 1, 0xDB
    emit 1, s
    exit
a_c:
    emit 1, 0xED
    emit 1, 78h
    exit
r_c:
    check dst eq dst element 1 & dst metadata 1 relativeto reg8
    jno err_arg
    check dst eq IXH | dst eq IXL | dst eq IYH | dst eq IYL
    jyes err_arg
    compute v, dst metadata 1 - reg8
    emit 1, 0xED
    emit 1, 01000000b or (v shl 3)
    exit
err_arg:
    err 'invalid argument'
end calminstruction

calminstruction OUT? args&
    local dst, src, d, v
    match dst=,src, args
    jno err_arg
    match (d), dst
    jno err_arg
    check src eq A
    jyes n_a
    check d eq C
    jyes c_r
    jump err_arg
n_a:
    check d eq C
    jyes c_a
    emit 1, 0xD3
    emit 1, d
    exit
c_a:
    emit 1, 0xED
    emit 1, 79h
    exit
c_r:
    check src = 0
    jyes c_0
    check src eq src element 1 & src metadata 1 relativeto reg8
    jno err_arg
    check src eq IXH | src eq IXL | src eq IYH | src eq IYL
    jyes err_arg
    compute v, src metadata 1 - reg8
    emit 1, 0xED
    emit 1, 01000001b or (v shl 3)
    exit
c_0:
    emit 1, 0xED
    emit 1, 71h
    exit
err_arg:
    err 'invalid argument'
end calminstruction

iterate <name, opcode>,\
        LDI,  0xA0, LDIR, 0xB0, LDD,  0xA8, LDDR, 0xB8,\
        CPI,  0xA1, CPIR, 0xB1, CPD,  0xA9, CPDR, 0xB9,\
        INI,  0xA2, INIR, 0xB2, IND,  0xAA, INDR, 0xBA,\
        OUTI, 0xA3, OTIR, 0xB3, OUTD, 0xAB, OTDR, 0xBB
    calminstruction name?
        emit 1, 0xED
        emit 1, opcode
    end calminstruction
end iterate
