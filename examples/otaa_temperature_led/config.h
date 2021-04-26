/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

// LoRaWAN region to use, full list of regions can be found at: 
//   http://stackforce.github.io/LoRaMac-doc/LoRaMac-doc-v4.5.1/group___l_o_r_a_m_a_c.html#ga3b9d54f0355b51e85df8b33fd1757eec
#define LORAWAN_REGION          LORAMAC_REGION_US915

// LoRaWAN Device EUI (64-bit), NULL value will use Default Dev EUI
#define LORAWAN_DEVICE_EUI      "0000000000000000"

// LoRaWAN Application / Join EUI (64-bit)
#define LORAWAN_APP_EUI         "0000000000000000"

// LoRaWAN Application Key (128-bit)
#define LORAWAN_APP_KEY         "00000000000000000000000000000000"

// LoRaWAN Channel Mask, NULL value will use the default channel mask 
// for the region
#define LORAWAN_CHANNEL_MASK    NULL
