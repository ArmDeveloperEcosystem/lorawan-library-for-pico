/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * This example uses OTAA to join the LoRaWAN network and then sends a
 * "hello world" uplink message periodically and prints out the
 * contents of any downlink message.
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/lorawan.h"
#include "tusb.h"

// edit with LoRaWAN Node Region and OTAA settings 
#include "config.h"

/*
 * Pin configuration for SX1276 radio module (default)
 */
const struct lorawan_sx12xx_settings sx12xx_settings = {
    .spi = {
        .inst = PICO_DEFAULT_SPI_INSTANCE,
        .mosi = PICO_DEFAULT_SPI_TX_PIN,
        .miso = PICO_DEFAULT_SPI_RX_PIN,
        .sck  = PICO_DEFAULT_SPI_SCK_PIN,
        .nss  = 8
    },
    .reset = 9,
    .dio0  = 7,
    .dio1  = 10
};

/*
 * Configuration for SX1262 Waveshare module
 */
//const struct lorawan_sx12xx_settings sx12xx_settings = {
//    .spi = {
//        .inst = spi1,
//        .mosi = 11,
//        .miso = 12,
//        .sck  = 10,
//        .nss  = 3
//    },
//    .reset = 15,
//    .busy = 2,
//    .dio1  = 20
//};

// OTAA settings
const struct lorawan_otaa_settings otaa_settings = {
    .device_eui   = LORAWAN_DEVICE_EUI,
    .app_eui      = LORAWAN_APP_EUI,
    .app_key      = LORAWAN_APP_KEY,
    .channel_mask = LORAWAN_CHANNEL_MASK
};

// variables for receiving data
int receive_length = 0;
uint8_t receive_buffer[242];
uint8_t receive_port = 0;

int main( void )
{
    // initialize stdio and wait for USB CDC connect
    stdio_init_all();

    while (!tud_cdc_connected()) {
        tight_loop_contents();
    }
    
    printf("Pico LoRaWAN - Hello OTAA\n\n");

    // uncomment next line to enable debug
    // lorawan_debug(true);

    // initialize the LoRaWAN stack
    printf("Initilizating LoRaWAN ... ");
    if (lorawan_init_otaa(&sx12xx_settings, LORAWAN_REGION, &otaa_settings) < 0) {
        printf("failed!!!\n");
        while (1) {
            tight_loop_contents();
        }
    } else {
        printf("success!\n");
    }

    // Start the join process and wait
    printf("Joining LoRaWAN network ... ");
    lorawan_join();

    while (!lorawan_is_joined()) {
        lorawan_process();
    }
    printf("joined successfully!\n");

    uint32_t last_message_time = 0;

    // loop forever
    while (1) {
        // let the lorwan library process pending events
        lorawan_process();

        // get the current time and see if 5 seconds have passed
        // since the last message was sent
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if ((now - last_message_time) > 5000) {
            const char* message = "hello world!";

            // try to send an unconfirmed uplink message
            printf("sending unconfirmed message '%s' ... ", message);
            if (lorawan_send_unconfirmed(message, strlen(message), 2) < 0) {
                printf("failed!!!\n");
            } else {
                printf("success!\n");
            }

            last_message_time = now;
        }

        // check if a downlink message was received
        receive_length = lorawan_receive(receive_buffer, sizeof(receive_buffer), &receive_port);
        if (receive_length > -1) {
            printf("received a %d byte message on port %d: ", receive_length, receive_port);

            for (int i = 0; i < receive_length; i++) {
                printf("%02x", receive_buffer[i]);
            }
            printf("\n");
        }
    }

    return 0;
}
