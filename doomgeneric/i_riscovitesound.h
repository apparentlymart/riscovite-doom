#ifndef __I_RISCOVITESOUND__
#define __I_RISCOVITESOUND__

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

#include <stdint.h>

extern uint64_t riscovite_sound_handle;

void riscovite_sound_interrupt_handler(uint64_t user_data, uint64_t buffer_space);

#endif
