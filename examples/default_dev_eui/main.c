/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * 
 * This example displays devices default LoRaWAN Dev EUI which is based
 * on the Pico SDK's pico_get_unique_board_id(...) API which uses the
 * on board NOR flash device 64-bit unique ID.
 * 
 * https://raspberrypi.github.io/pico-sdk-doxygen/group__pico__unique__id.html
 * 
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/lorawan.h"
#include "tusb.h"

int main(void)
{
    char devEui[17];

    // initialize stdio and wait for USB CDC connect
    stdio_init_all();

    while (!tud_cdc_connected()) {
        tight_loop_contents();
    }
    
    // get the default Dev EUI as a string and print it out
    printf("Pico LoRaWAN - Default Dev EUI = %s\n", lorawan_default_dev_eui(devEui));

    // do nothing
    while (1) {
        tight_loop_contents();
    }

    return 0;
}
