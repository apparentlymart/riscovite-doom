//
// Copyright(C) 2025 Martin Atkins
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	RISCovite-specific sound implementation.
//

#include "config.h"
#include "doomtype.h"
#include "i_sound.h"
#include "i_riscovitesound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "deh_str.h"

#include <stdio.h>
#include <riscovite.h>

uint64_t riscovite_sound_handle = 0; // should be changed by the early init code

static boolean use_sfx_prefix;

static boolean I_Riscovite_InitSound(boolean _use_sfx_prefix) {
    // The early init code should've set riscovite_sound_handle to something
    // nonzero after acquiring the BGAI handle. If not then we'll just
    // disable sound completely.
    if (riscovite_sound_handle == 0) {
        printf("I_Riscovite_InitSound: sound was not initialized, so disabling sound module\n");
    }

    printf("I_Riscovite_InitSound: starting RISCovite sound output\n");
    use_sfx_prefix = _use_sfx_prefix;
    return true;
}

static void I_Riscovite_ShutdownSound(void) {
}

static int I_Riscovite_GetSfxLumpNum(sfxinfo_t *sfx) {
    char namebuf[9];
    if (sfx->link != NULL) {
        sfx = sfx->link;
    }
    if (use_sfx_prefix) {
        snprintf(namebuf, sizeof(namebuf), "ds%s", DEH_String(sfx->name));
    } else {
        M_StringCopy(namebuf, DEH_String(sfx->name), sizeof(namebuf));
    }
    return W_GetNumForName(namebuf);
}

static void I_Riscovite_UpdateSound(void) {
}

static void I_Riscovite_UpdateSoundParams(int handle, int vol, int sep) {
}

static int I_Riscovite_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep) {
    return 0;
}

static void I_Riscovite_StopSound(int handle) {
}

static boolean I_Riscovite_SoundIsPlaying(int handle) {
    return false;
}

static void I_Riscovite_PrecacheSounds(sfxinfo_t *sounds, int num_sounds) {
}

static snddevice_t sound_riscovite_devices[] = 
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_AWE32,
};

sound_module_t DG_sound_module = 
{
    sound_riscovite_devices,
    arrlen(sound_riscovite_devices),
    I_Riscovite_InitSound,
    I_Riscovite_ShutdownSound,
    I_Riscovite_GetSfxLumpNum,
    I_Riscovite_UpdateSound,
    I_Riscovite_UpdateSoundParams,
    I_Riscovite_StartSound,
    I_Riscovite_StopSound,
    I_Riscovite_SoundIsPlaying,
    I_Riscovite_PrecacheSounds,
};

#define SYS_WRITE_SAMPLES(slot) (~((slot) | (0x00000001 << 4)))

static inline struct riscovite_result_void
write_samples(uint64_t hnd, uint16_t *samples, uint64_t count) {
    register uint64_t _hnd asm("a0") = hnd;
    register uint64_t _samples asm("a1") = (uint64_t)samples;
    register uint64_t _count asm("a2") = (uint64_t)count;
    register uint64_t __ret asm("a0");
    register uint64_t __err asm("a1");
    asm volatile inline(
        "li a7, %[FUNCNUM]\n"
        "ecall\n"
        : [RET] "=r"(__ret), [ERR] "=r"(__err)
        : "r0"(_hnd), "r1"(_samples), "r"(_count),
            [FUNCNUM] "n"(SYS_WRITE_SAMPLES(1))
        : "a7", "memory");
    return ((struct riscovite_result_void){__ret, __err});
}

#include <math.h>

// This function periodically interrupts the rest of the program to feed
// the RISCovite sound output buffer.
void riscovite_sound_interrupt_handler(uint64_t user_data, uint64_t buffer_space) {
    // TEMP: For now we just produce a sine wave to establish that we're
    // able to get interrupted here at all.
    const double SAMPLE_RATE = 44100.0;
    static int16_t SAMPLE_BUF[4096] = {0};
    static double SAMPLE_CLOCK = 0.0F;

    if (buffer_space > (sizeof(SAMPLE_BUF) / sizeof(SAMPLE_BUF[0]))) {
        buffer_space = sizeof(SAMPLE_BUF) / sizeof(SAMPLE_BUF[0]);
    }
    uint64_t frame_count = buffer_space / 2;

    int16_t(*samples)[2] = (int16_t(*)[2])SAMPLE_BUF;
    for (int i = 0; i < frame_count; i++) {
        SAMPLE_CLOCK = SAMPLE_CLOCK + 1.0;
        while (SAMPLE_CLOCK > SAMPLE_RATE) {
            SAMPLE_CLOCK -= SAMPLE_RATE;
        }
        double next_sample = sin((SAMPLE_CLOCK * 440.0 * 2.0 * 3.14159265358979323846 / SAMPLE_RATE));
        double sample_raw = (int16_t)(next_sample * 32767.0);

        (*samples)[0] = sample_raw;  // Left channel
        (*samples)[1] = -sample_raw; // Right channel

        samples++;
    }
    write_samples(riscovite_sound_handle, &SAMPLE_BUF[0], sizeof(SAMPLE_BUF) / sizeof(SAMPLE_BUF[0]));
}
