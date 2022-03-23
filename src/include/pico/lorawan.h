/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#ifndef _PICO_LORAWAN_H_
#define _PICO_LORAWAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "LoRaMac.h"

struct lorawan_sx1276_settings {
    struct {
        spi_inst_t* inst;
        uint mosi;
        uint miso;
        uint sck;
        uint nss;
    } spi;
    uint reset;
    uint dio0;
    uint dio1;
};

struct lorawan_abp_settings {
    const char* device_address;
    const char* network_session_key;
    const char* app_session_key;
    const char* channel_mask;
};

struct lorawan_otaa_settings {
    const char* device_eui;
    const char* app_eui;
    const char* app_key;
    const char* channel_mask;
};

const char* lorawan_default_dev_eui(char* dev_eui);

int lorawan_init(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region);

int lorawan_init_abp(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region, const struct lorawan_abp_settings* abp_settings);

int lorawan_init_otaa(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region, const struct lorawan_otaa_settings* otaa_settings);

int lorawan_join();

int lorawan_is_joined();

int lorawan_process();

int lorawan_process_timeout_ms(uint32_t timeout_ms);

int lorawan_send_unconfirmed(const void* data, uint8_t data_len, uint8_t app_port);

int lorawan_receive(void* data, uint8_t data_len, uint8_t* app_port);

void lorawan_debug(bool debug);

int lorawan_erase_nvm();

#ifdef __cplusplus
}
#endif

#endif
