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
//	RISCovite-specific music implementation.
//

#include "config.h"
#include "doomtype.h"
#include "i_sound.h"
#include "i_riscovitesound.h"

#include <stdio.h>

static boolean I_Riscovite_InitMusic(void) {
    // The early init code should've set riscovite_sound_handle to something
    // nonzero after acquiring the BGAI handle. If not then we'll just
    // disable music completely.
    if (riscovite_sound_handle == 0) {
        printf("I_Riscovite_InitMusic: sound was not initialized, so disabling music module\n");
    }

    printf("I_Riscovite_InitMusic: starting RISCovite music output\n");
    return true;
}

static void I_Riscovite_ShutdownMusic(void) {
}

static void I_Riscovite_SetMusicVolume(int volume) {
}

static void I_Riscovite_PauseSong(void) {
}

static void I_Riscovite_ResumeSong(void) {
}

static void *I_Riscovite_RegisterSong(void *data, int len) {
    return (void *)0;
}

static void I_Riscovite_UnRegisterSong(void *handle) {
}

static void I_Riscovite_PlaySong(void *handle, boolean looping) {
}

static void I_Riscovite_StopSong(void) {
}

static boolean I_Riscovite_MusicIsPlaying(void) {
    return false;
}

static void I_Riscovite_PollMusic(void) {
}

static snddevice_t music_riscovite_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

music_module_t DG_music_module =
{
    music_riscovite_devices,
    arrlen(music_riscovite_devices),
    I_Riscovite_InitMusic,
    I_Riscovite_ShutdownMusic,
    I_Riscovite_SetMusicVolume,
    I_Riscovite_PauseSong,
    I_Riscovite_ResumeSong,
    I_Riscovite_RegisterSong,
    I_Riscovite_UnRegisterSong,
    I_Riscovite_PlaySong,
    I_Riscovite_StopSong,
    I_Riscovite_MusicIsPlaying,
    I_Riscovite_PollMusic,
};
