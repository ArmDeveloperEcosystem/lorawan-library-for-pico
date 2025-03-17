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
static uint8_t eeprom_in_use = 0; // used to track if the EEPROM is being accessed or not

uint8_t EepromInUse()
{
    if (eeprom_in_use) {
        return (1);
    } else {
        return (0);
    }
}

void EepromMcuInit()
{
    eeprom_in_use = 1;
    memcpy(eeprom_write_cache, EEPROM_ADDRESS, sizeof(eeprom_write_cache));
    eeprom_in_use = 0;
}

uint8_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    eeprom_in_use = 1;
    memcpy(buffer, eeprom_write_cache + addr, size);
    eeprom_in_use = 0;
    
    return SUCCESS;
}

uint8_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    eeprom_in_use = 1;
    memcpy(eeprom_write_cache + addr, buffer, size);
    eeprom_in_use = 0;

    return SUCCESS;
}

uint8_t EepromMcuFlush()
{
    uint32_t mask;

    eeprom_in_use = 1;
    BoardCriticalSectionBegin(&mask);

    flash_range_erase(EEPROM_OFFSET, sizeof(eeprom_write_cache));
    flash_range_program(EEPROM_OFFSET, eeprom_write_cache, sizeof(eeprom_write_cache));

    BoardCriticalSectionEnd(&mask);
    eeprom_in_use = 0;
}
