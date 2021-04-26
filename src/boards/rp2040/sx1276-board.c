/*!
 * \file      sx1276-board.c
 *
 * \brief     Target board SX1276 driver implementation
 * 
 * \remark    This is based on 
 *            https://github.com/Lora-net/LoRaMac-node/blob/master/src/boards/B-L072Z-LRWAN1/sx1276-board.c
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 * 
 */

#include <stddef.h>

#include "hardware/gpio.h"

#include "delay.h"
#include "sx1276-board.h"

#include "radio/radio.h"

const struct Radio_s Radio =
{
    SX1276Init,
    SX1276GetStatus,
    SX1276SetModem,
    SX1276SetChannel,
    SX1276IsChannelFree,
    SX1276Random,
    SX1276SetRxConfig,
    SX1276SetTxConfig,
    SX1276CheckRfFrequency,
    SX1276GetTimeOnAir,
    SX1276Send,
    SX1276SetSleep,
    SX1276SetStby,
    SX1276SetRx,
    SX1276StartCad,
    SX1276SetTxContinuousWave,
    SX1276ReadRssi,
    SX1276Write,
    SX1276Read,
    SX1276WriteBuffer,
    SX1276ReadBuffer,
    SX1276SetMaxPayloadLength,
    SX1276SetPublicNetwork,
    SX1276GetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

static DioIrqHandler** irq_handlers;

void dio_gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == SX1276.DIO0.pin) {
        irq_handlers[0](NULL);
    } else if (gpio == SX1276.DIO1.pin) {
        irq_handlers[1](NULL);
    }
}

void SX1276SetAntSwLowPower( bool status )
{
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    return true;
}

void SX1276SetBoardTcxo( uint8_t state )
{
}

uint32_t SX1276GetDio1PinState( void )
{
    return GpioRead(&SX1276.DIO1);
}

void SX1276SetAntSw( uint8_t opMode )
{
}

void SX1276Reset( void )
{
    GpioInit( &SX1276.Reset, SX1276.Reset.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 ); // RST

    DelayMs (1);

    GpioInit( &SX1276.Reset, SX1276.Reset.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 ); // RST

    DelayMs (6);
}

void SX1276IoInit( void )
{
    GpioInit( &SX1276.Spi.Nss, SX1276.Spi.Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 ); // CS
    GpioInit( &SX1276.Reset, SX1276.Reset.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );     // RST

    GpioInit( &SX1276.DIO0, SX1276.DIO0.pin, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );        // IRQ / DIO0
    GpioInit( &SX1276.DIO1, SX1276.DIO1.pin, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );        // DI01
}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{
    irq_handlers = irqHandlers;

    gpio_set_irq_enabled_with_callback(SX1276.DIO0.pin, GPIO_IRQ_EDGE_RISE, true, &dio_gpio_callback);
    gpio_set_irq_enabled_with_callback(SX1276.DIO1.pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &dio_gpio_callback);
}

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] power Selects the right PA according to the wanted power.
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1276GetPaSelect( int8_t power );

void SX1276SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( power );

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power > 0 )
        {
            if( power > 15 )
            {
                power = 15;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 7 << 4 ) | ( power );
        }
        else
        {
            if( power < -4 )
            {
                power = -4;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 0 << 4 ) | ( power + 4 );
        }
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );
}

static uint8_t SX1276GetPaSelect( int8_t power )
{
    if( power > 14 )
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
}

uint32_t SX1276GetBoardTcxoWakeupTime( void )
{
    return 0;
}
