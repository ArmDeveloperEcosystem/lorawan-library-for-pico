/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include "hardware/timer.h"

#include "delay-board.h"

void DelayMsMcu( uint32_t ms )
{
    busy_wait_us_32(ms * 1000);
}
