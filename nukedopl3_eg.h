/*
 * Copyright (C) 2013-2018 Alexey Khokholov (Nuke.YKT)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *  Nuked OPL3 emulator.
 *  Thanks:
 *      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *          Feedback and Rhythm part calculation information.
 *      forums.submarine.org.uk(carbon14, opl3):
 *          Tremolo and phase generator calculation information.
 *      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *          OPL2 ROMs.
 *      siliconpr0n.org(John McMaster, digshadow):
 *          YMF262 and VRC VII decaps and die shots.
 *
 * version: 1.8
 */

#ifndef OPL_OPL3_H
#define OPL_OPL3_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t       Bitu;
typedef intptr_t        Bits;
typedef uint64_t        Bit64u;
typedef int64_t         Bit64s;
typedef uint32_t        Bit32u;
typedef int32_t         Bit32s;
typedef uint16_t        Bit16u;
typedef int16_t         Bit16s;
typedef uint8_t         Bit8u;
typedef int8_t          Bit8s;

struct opl3_chip
{
    // chip
    Bit16u timer;
    Bit64u eg_timer;
    Bit8u eg_timerrem;
    Bit8u eg_state;
    Bit8u eg_add;
    Bit8u trem;
    // channel
    Bit16u f_num;  // 0-1023
    Bit8u block;  // 0-7
    Bit8u ksv;
    // slot
    Bit16s eg_rout;
    Bit16s eg_out;
    Bit8u eg_inc;
    Bit8u eg_gen;
    Bit8u eg_rate;
    Bit8u eg_ksl;
    Bit8u reg_type;
    Bit8u reg_ksr;
    Bit8u reg_ksl;
    Bit8u reg_tl;
    Bit8u reg_ar;
    Bit8u reg_dr;
    Bit8u reg_sl;
    Bit8u reg_rr;
    Bit8u key;
};

enum {
    egk_norm = 0x01,
    egk_drum = 0x02
};

enum envelope_gen_num
{
    envelope_gen_num_attack = 0,
    envelope_gen_num_decay = 1,
    envelope_gen_num_sustain = 2,
    envelope_gen_num_release = 3
};

extern const Bit8u kslrom[16];
extern const Bit8u kslshift[4];

void OPL3_EnvelopeUpdateKSL(opl3_chip *chip);
void OPL3_EnvelopeCalc(opl3_chip *chip);
void OPL3_EnvelopeKeyOn(opl3_chip *chip, Bit8u type);
void OPL3_EnvelopeKeyOff(opl3_chip *chip, Bit8u type);
void OPL3_Tick(opl3_chip *chip);
void OPL3_Reset(opl3_chip *chip);

#ifdef __cplusplus
}
#endif

#endif
