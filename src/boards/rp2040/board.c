/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include <stdint.h>
#include <string.h>

#include "pico.h"
#include "pico/unique_id.h"
#include "hardware/sync.h"

#include "board.h"

void BoardInitMcu( void )
{
}

void BoardInitPeriph( void )
{
}

void BoardLowPowerHandler( void )
{
    __wfi();
}

uint8_t BoardGetBatteryLevel( void )
{
    return 0;
}

uint32_t BoardGetRandomSeed( void )
{
    uint8_t id[8];

    BoardGetUniqueId(id);

    return (id[3] << 24) | (id[2] << 16) | (id[1] << 1) | id[0];
}

void BoardGetUniqueId( uint8_t *id )
{
    pico_unique_board_id_t board_id;
    
    pico_get_unique_board_id(&board_id);

    memcpy(id, board_id.id, 8);
}

void BoardCriticalSectionBegin( uint32_t *mask )
{
    *mask = save_and_disable_interrupts();
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    restore_interrupts(*mask);
}

void BoardResetMcu( void )
{
}
