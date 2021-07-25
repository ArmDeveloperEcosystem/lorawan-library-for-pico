/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico.h"
#include "pico/stdlib.h"
#include "utilities.h"
#include "eeprom-board.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define USER_CODE_LEN                   (FLASH_PAGE_SIZE * 1024)                        // 256kB for user code
#define USER_DATA_LEN                   (FLASH_SECTOR_SIZE * 4)                         // 4kB for user data
#define LORAWAN_NVM_BASE_ADDR           (XIP_BASE + USER_CODE_LEN + USER_DATA_LEN)      // use for store lorawan nvm data, which is more than 2kB
#define LORAWAN_NVM_BASE_OFFSET         (USER_CODE_LEN + USER_DATA_LEN)

const uint8_t *nvm_target_contents = (const uint8_t *) LORAWAN_NVM_BASE_ADDR;
static uint8_t nvm_cache[FLASH_SECTOR_SIZE];

void erase_lorawan_nvm_flash_sector(void);
void cache_lorawan_nvm_flash_sector(uint8_t *sector_data);
void write_lorawan_nvm_flash_sector(uint8_t *sector_data);

LmnStatus_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    LmnStatus_t status = LMN_STATUS_ERROR;

    cache_lorawan_nvm_flash_sector(nvm_cache);          // cache
    erase_lorawan_nvm_flash_sector();                   // erase
    memcpy((void *)(nvm_cache + addr), buffer, size);   // update
    write_lorawan_nvm_flash_sector(nvm_cache);          // write
    status = LMN_STATUS_OK;

    return status;
}

LmnStatus_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    LmnStatus_t status = LMN_STATUS_OK;

    memcpy( buffer, ( uint8_t* )( LORAWAN_NVM_BASE_ADDR + addr ), size );

    return status;
}


void erase_lorawan_nvm_flash_sector(void)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(LORAWAN_NVM_BASE_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
}

void cache_lorawan_nvm_flash_sector(uint8_t *sector_data)
{
    for(int i=0; i<FLASH_SECTOR_SIZE; i++) {
        sector_data[i] = nvm_target_contents[i];
    }
}

void write_lorawan_nvm_flash_page(uint16_t page_num, uint8_t *page_data)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(LORAWAN_NVM_BASE_OFFSET + page_num * FLASH_PAGE_SIZE, page_data, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

void write_lorawan_nvm_flash_sector(uint8_t *sector_data)
{
    for(int i=0; i<16; i++)
        write_lorawan_nvm_flash_page(i, sector_data + FLASH_PAGE_SIZE * i);
}

void print_nvm_flash_buf(const uint8_t *buf, size_t len) 
{
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

void print_lorawan_nvm_flash_sector(void)
{
    printf("LoRaWan NVM data: \n");
    for(int i=0; i<16; i++) print_nvm_flash_buf(nvm_target_contents + FLASH_PAGE_SIZE * i, FLASH_PAGE_SIZE);
}
