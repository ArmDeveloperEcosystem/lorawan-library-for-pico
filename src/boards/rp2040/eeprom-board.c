/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"

#include "utilities.h"
#include "eeprom-board.h"

#define EEPROM_SIZE    (FLASH_SECTOR_SIZE)
#define EEPROM_OFFSET  (PICO_FLASH_SIZE_BYTES - EEPROM_SIZE)
#define EEPROM_ADDRESS ((const uint8_t*)(XIP_BASE + EEPROM_OFFSET))

static uint8_t eeprom_write_cache[EEPROM_SIZE];

void EepromMcuInit()
{
    memcpy(eeprom_write_cache, EEPROM_ADDRESS, sizeof(eeprom_write_cache));
}

uint8_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    memcpy(buffer, eeprom_write_cache + addr, size);
    
    return SUCCESS;
}

uint8_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    memcpy(eeprom_write_cache + addr, buffer, size);

    return SUCCESS;
}

uint8_t EepromMcuFlush()
{
    uint32_t mask;

    BoardCriticalSectionBegin(&mask);

    flash_range_erase(EEPROM_OFFSET, sizeof(eeprom_write_cache));
    flash_range_program(EEPROM_OFFSET, eeprom_write_cache, sizeof(eeprom_write_cache));

    BoardCriticalSectionEnd(&mask);
}
