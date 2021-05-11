/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include "pico/time.h"
#include "pico/stdlib.h"

#include "rtc-board.h"

static alarm_pool_t* rtc_alarm_pool = NULL;
static absolute_time_t rtc_timer_context;
static alarm_id_t last_rtc_alarm_id = -1;

void RtcInit( void )
{
    rtc_alarm_pool = alarm_pool_create(2, 16);

    RtcSetTimerContext();
}

uint32_t RtcGetCalendarTime( uint16_t *milliseconds )
{
    uint32_t now = to_ms_since_boot(get_absolute_time());

    *milliseconds = (now % 1000);

    return (now / 1000);
}

void RtcBkupRead( uint32_t *data0, uint32_t *data1 )
{
    *data0 = 0;
    *data1 = 0;
}

uint32_t RtcGetTimerElapsedTime( void )
{
    int64_t delta = absolute_time_diff_us(rtc_timer_context, get_absolute_time());

    return delta;
}

uint32_t RtcSetTimerContext( void )
{
    rtc_timer_context = get_absolute_time();

    uint64_t ticks = to_us_since_boot(rtc_timer_context);

    return ticks;
}

uint32_t RtcGetTimerContext( void )
{
    uint64_t ticks = to_us_since_boot(rtc_timer_context);

    return ticks;
}

uint32_t RtcGetMinimumTimeout( void )
{
    return 1;
}

static int64_t alarm_callback(alarm_id_t id, void *user_data) {
    TimerIrqHandler( );

    return 0;
}

void RtcSetAlarm( uint32_t timeout )
{
    if (last_rtc_alarm_id > -1) {
        alarm_pool_cancel_alarm(rtc_alarm_pool, last_rtc_alarm_id);
    }

    last_rtc_alarm_id = alarm_pool_add_alarm_at(rtc_alarm_pool, delayed_by_us(rtc_timer_context, timeout), alarm_callback, NULL, true);
}

void RtcStopAlarm( void )
{
    if (last_rtc_alarm_id > -1) {
        alarm_pool_cancel_alarm(rtc_alarm_pool, last_rtc_alarm_id);
    }
}

uint32_t RtcMs2Tick( TimerTime_t milliseconds )
{
    return milliseconds * 1000;
}

uint32_t RtcGetTimerValue( void )
{
    uint64_t now = to_us_since_boot(get_absolute_time());

    return now;
}

TimerTime_t RtcTick2Ms( uint32_t tick )
{
    return us_to_ms(tick);
}

void RtcBkupWrite( uint32_t data0, uint32_t data1 )
{
}

void RtcProcess( void )
{
    // Not used on this platform.
}
