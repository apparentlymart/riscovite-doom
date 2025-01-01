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
#include "i_system.h"
#include "m_misc.h"
#include "w_wad.h"
#include "deh_str.h"
#include "z_zone.h"

#include <stdio.h>
#include <stdint.h>
#include <riscovite.h>
#include <stdlib.h>
#include <string.h>

uint64_t riscovite_sound_handle = 0; // should be changed by the early init code

static boolean use_sfx_prefix;

struct sound_channel {
    uintptr_t next_addr;
    uintptr_t stop_addr;
    sfxinfo_t *sfxinfo; // null when channel is inactive
    int addr_shift;
    double left_vol;
    double right_vol;
};

#define NUM_CHANNELS (16)

static struct sound_channel sound_channels[NUM_CHANNELS] = {0};

// struct cached_sound is the type we use for sfxinfo_t.driver_data when
// we're the active sound module.
struct cached_sound {
    uint8_t *samples;
    int length;
    int ref_count;
    int addr_shift;
};

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

static int get_sfx_lump_num(sfxinfo_t *sfx) {
    char namebuf[9];
    if (sfx->link != NULL) {
        sfx = sfx->link;
    }
    if (use_sfx_prefix) {
        snprintf(namebuf, sizeof(namebuf), "ds%s", DEH_String(sfx->name));
    } else {
        M_StringCopy(namebuf, DEH_String(sfx->name), sizeof(namebuf));
    }
    return W_CheckNumForName(namebuf);
}

static int I_Riscovite_GetSfxLumpNum(sfxinfo_t *sfx) {
    int ret = get_sfx_lump_num(sfx);
    if (ret == -1) {
        I_Error("I_Riscovite_GetSfxLumpNum: %s not found!", sfx->name);
    }
    return ret;
}

static void I_Riscovite_UpdateSound(void) {
}

static void I_Riscovite_UpdateSoundParams(int handle, int vol, int sep) {
    if (handle < 0 || handle >= NUM_CHANNELS) {
        return;
    }
    struct sound_channel *channel = &sound_channels[handle];

    int left = ((254 - sep) * vol) / 127;
    int right = ((sep) * vol) / 127;

    if (left < 0) left = 0;
    else if ( left > 255) left = 255;
    if (right < 0) right = 0;
    else if (right > 255) right = 255;

    // NOTE: This is a little risky, because the interrupt handler could
    // run at any point during the following assignments. We're assuming
    // it's okay because the panning/volume is likely to change gradually
    // and so not a big deal if one of these changes gets observed before
    // the other one.
    channel->left_vol = (double)left / 255.0;
    channel->right_vol = (double)right / 255.0;
}

static struct cached_sound *load_sound(sfxinfo_t *sfxinfo) {
    struct cached_sound *ret = NULL;

    int lumpnum = sfxinfo->lumpnum;
    uint8_t *raw = W_CacheLumpNum(lumpnum, PU_STATIC);
    if (raw == NULL) {
        return NULL;
    }
    // After this point we must always return indirectly with "goto done"
    // so that we'll release the cached lump data.

    int raw_len = W_LumpLength(lumpnum);
    if (raw_len < 8 || raw[0] != 0x03 || raw[1] != 0x00) {
        // Invalid sound data
        goto done;
    }

    int sample_rate = (raw[3] << 8) | raw[2];
    int length = (raw[7] << 24) | (raw[6] << 16) | (raw[5] << 8) | raw[4];
    if (length > (raw_len - 8) || length <= 48) {
        // Length is out of bounds, so this is invalid sound data
        goto done;
    }

    // The original sound library used by Doom always ignores the first and
    // last 16 bytes of the data, so we'll mimic that here.
    raw += 16;
    length -= 32;

    // We'll allocate a single block of memory that includes both our
    // struct cached_sound and the raw sample data afterwards, just so that
    // we have one fewer allocation to keep track of.
    ret = malloc(sizeof(struct cached_sound) + length);
    if (ret == NULL) {
        goto done;
    }
    ret->ref_count = 0;
    ret->samples = (uint8_t *)(&ret[1]); // points to the byte immediately after the cached_sound
    ret->length = length;
    memcpy(ret->samples, raw, length); // Copy the sample data into our own allocation

    // We'll precalculate the address shift based on the sample rate. Shifting
    // the address is how we handle sample rate conversions, by treating each
    // channel's address field as a fixed point fraction with either two, one,
    // or zero fractional parts.
    // RISCovite always uses 44100Hz, so shift of zero would represent that rate.
    switch (sample_rate) {
    case 11025:
        ret->addr_shift = 2;
        break;
    case 22050:
        ret->addr_shift = 1;
        break;
    default:
        // We should not get here because all samples in Doom use one of the
        // two sample rates above. Using this port with a custom WAD that uses
        // different sample rates will cause an incorrect playback speed.
        ret->addr_shift = 0;
    }

done:
    W_ReleaseLumpNum(lumpnum);
    return ret;
}

static boolean lock_sound(sfxinfo_t *sfxinfo) {
    if (sfxinfo->driver_data == NULL) {
        // This is a sample we've not seen before, so we'll need to load
        // its sample data into memory.
        sfxinfo->driver_data = load_sound(sfxinfo);
        if (sfxinfo->driver_data == NULL) {
            return false;
        }
    }

    struct cached_sound *cached = sfxinfo->driver_data;
    cached->ref_count++;
    return true;
}

static inline sfxinfo_t *stop_sound(int handle) {
    // Setting sfxinfo to NULL is enough to make the audio interrupt ignore
    // this channel entirely, until I_Riscovite_StartSound writes a non-NULL
    // pointer here again later.
    return __atomic_exchange_n(&sound_channels[handle].sfxinfo, NULL, __ATOMIC_SEQ_CST);
}

static int I_Riscovite_StartSound(sfxinfo_t *sfxinfo, int channel_num, int vol, int sep) {
    // This function could be interrupted at any point by
    // riscovite_sound_interrupt_handler, and so we use each channel's sfxinfo
    // pointer to represent ownership of the other fields of the channel:
    // - when sfxinfo is null, the interrupt handler ignores the other fields
    //   entirely and so this function can modify them.
    // - when sfxinfo is not null, the interrupt handler owns all of the other
    //   fields and so our only valid operation is to set sfxinfo to null so
    //   we can reclaim ownership.
    // The interrupt handler has a higher priority than this function, so this
    // function cannot possibly interrupt the interrupt handler.

    if (channel_num < 0 || channel_num >= NUM_CHANNELS) {
        return -1;
    }

    struct sound_channel *channel = &sound_channels[channel_num];
    stop_sound(channel_num);
    // If the interrupt handler is triggered from here to when we store a
    // new pointer into channel->sfxinfo then it will treat this channel as
    // inactive, so we can safely modify its other fields.

    if (!lock_sound(sfxinfo)) {
        return -1;
    }
    struct cached_sound *cached = sfxinfo->driver_data;

    // The following address shifting is how we implement sample rate conversion:
    // we effectively treat the channel's address fields as fixed-point fractions
    // so that we can stretch out samples as needed to convert to RISCovite's
    // higher sample rate.
    channel->addr_shift = cached->addr_shift;
    channel->next_addr = (uintptr_t)(cached->samples) << cached->addr_shift;
    channel->stop_addr = (uintptr_t)(cached->samples + cached->length) << cached->addr_shift;
    I_Riscovite_UpdateSoundParams(channel_num, vol, sep); // sets vol_left and vol_right

    printf("playing %s on channel %d with vol=%d, sep=%d\n", sfxinfo->name, channel_num, vol, sep);

    // We'll now finally populate sfxinfo, which makes this channel active
    // as far as the sound interrupt is concerned.
    __atomic_store_n(&channel->sfxinfo, sfxinfo, __ATOMIC_SEQ_CST);

    return channel_num;
}

static void I_Riscovite_StopSound(int handle) {
    stop_sound(handle);
}

static boolean I_Riscovite_SoundIsPlaying(int handle) {
    if (handle < 0 || handle >= NUM_CHANNELS) {
        return false;
    }
    struct sound_channel *channel = &sound_channels[handle];
    sfxinfo_t *existing = __atomic_load_n(&channel->sfxinfo, __ATOMIC_SEQ_CST);
    return existing != NULL;
}

static void I_Riscovite_PrecacheSounds(sfxinfo_t *sounds, int num_sounds) {
    sfxinfo_t *stop = sounds + num_sounds;
    for (sfxinfo_t *sound = sounds; sound != stop; sound++) {
        int lump_num = get_sfx_lump_num(sound);
        if (lump_num < 0) {
            continue;
        }
        sound->lumpnum = lump_num;
        sound->driver_data = load_sound(sound);
    }
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

struct sound_frame {
    int16_t left;
    int16_t right;
};

static inline uint64_t raw_sample(double s) {
    s *= 32767.0;
    if (s > 32767.0) {
        s = 32767.0;
    } else if (s < -32767.0) {
        s = -32767.0;
    }
    return (int16_t)s;
}

// This function periodically interrupts the rest of the program to feed
// the RISCovite sound output buffer.
void riscovite_sound_interrupt_handler(uint64_t user_data, uint64_t buffer_space) {
    static int16_t SAMPLE_BUF[4096] = {0};
    const double sample_scale = 0.65; // Arbitrary scale factor to give some headroom for mixing

    if (buffer_space > (sizeof(SAMPLE_BUF) / sizeof(SAMPLE_BUF[0]))) {
        buffer_space = sizeof(SAMPLE_BUF) / sizeof(SAMPLE_BUF[0]);
    }
    uint64_t frame_count = buffer_space / 2;

    struct sound_frame *samples = (struct sound_frame *)SAMPLE_BUF;
    for (int fi = 0; fi < frame_count; fi++) {
        double left = 0.0;
        double right = 0.0;

        struct sound_channel *channel = &sound_channels[0];
        for (int ci = 0; ci < NUM_CHANNELS; ci++) {
            sfxinfo_t *info = __atomic_load_n(&channel->sfxinfo, __ATOMIC_SEQ_CST);
            if (info == NULL) {
                continue; // channel is currently inactive, so we mustn't modify it at all
            }

            uintptr_t addr = channel->next_addr;
            uint8_t *src_sample = (uint8_t *)(addr >> channel->addr_shift);
            addr++;
            if (addr == channel->stop_addr) {
                // Activity on this channel is finished.
                __atomic_store_n(&channel->sfxinfo, NULL, __ATOMIC_SEQ_CST);
            } else {
                channel->next_addr = addr;
            }

            double scaled = (((double)*src_sample) - 128) * sample_scale / 127.0;
            left += (scaled  * channel->left_vol);
            right += (scaled  * channel->right_vol);

            channel++;
        }

        // TODO: Mix in a sample from the music buffer too, if any.

        samples->left = raw_sample(left);
        samples->right = raw_sample(right);
        samples++;
    }
    write_samples(riscovite_sound_handle, &SAMPLE_BUF[0], buffer_space);
}
