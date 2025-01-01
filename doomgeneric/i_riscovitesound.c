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

#include <stdio.h>

static boolean I_Riscovite_InitSound(boolean _use_sfx_prefix) {
    printf("I_Riscovite_InitSound: starting RISCovite sound output\n");
    return true;
}

static void I_Riscovite_ShutdownSound(void) {
}

static int I_Riscovite_GetSfxLumpNum(sfxinfo_t *sfx) {
    return 0;
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
