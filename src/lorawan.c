/*!
 * \file      lorawan.c
 *
 * \brief     Implements the LoRaMac layer handling. 
 *            Provides the possibility to register applicative packages.
 *
 * \remark    This file is based on
 *            https://github.com/Lora-net/LoRaMac-node/blob/master/src/apps/LoRaMac/periodic-uplink-lpp/B-L072Z-LRWAN1/main.c
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
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 */

#include <stdio.h>
#include <string.h>

#include "pico/lorawan.h"
#include "pico/time.h"

#include "board.h"
#include "rtc-board.h"
#include "sx1276-board.h"

#include "../../periodic-uplink-lpp/firmwareVersion.h"
#include "Commissioning.h"
#include "RegionCommon.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "LmHandlerMsgDisplay.h"
#include "NvmDataMgmt.h"

/*!
 * LoRaWAN default end-device class
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_0

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                        true

/*!
 * Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK                      true

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData =
{
    .Buffer = AppDataBuffer,
    .BufferSize = 0,
    .Port = 0,
};

static void OnMacProcessNotify( void );
static void OnNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );
static void OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t* params );
#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection );
#else
static void OnSysTimeUpdate( void );
#endif

static void OnTxPeriodicityChanged( uint32_t periodicity );
static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed );
static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity );

static LmHandlerCallbacks_t LmHandlerCallbacks =
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    .GetTemperature = NULL,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNvmDataChange = OnNvmDataChange,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange= OnClassChange,
    .OnBeaconStatusChange = OnBeaconStatusChange,
    .OnSysTimeUpdate = OnSysTimeUpdate,
};

static LmHandlerParams_t LmHandlerParams =
{
    .Region = ACTIVE_REGION,
    .AdrEnable = LORAWAN_ADR_STATE,
    .IsTxConfirmed = LORAWAN_DEFAULT_CONFIRMED_MSG_STATE,
    .TxDatarate = LORAWAN_DEFAULT_DATARATE,
    .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
    .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer = AppDataBuffer,
    .PingSlotPeriodicity = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

static LmhpComplianceParams_t LmhpComplianceParams =
{
    .FwVersion.Value = FIRMWARE_VERSION,
    .OnTxPeriodicityChanged = OnTxPeriodicityChanged,
    .OnTxFrameCtrlChanged = OnTxFrameCtrlChanged,
    .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint32_t TxPeriodicity = 0;

static const struct lorawan_abp_settings* AbpSettings = NULL;

static const struct lorawan_otaa_settings* OtaaSettings = NULL;

static uint8_t AppRxDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

static LmHandlerAppData_t AppRxData =
{
    .Buffer = AppRxDataBuffer,
    .BufferSize = 0,
    .Port = 0,
};

static bool Debug = false;

extern void EepromMcuInit();
extern uint8_t EepromMcuFlush();

const char* lorawan_default_dev_eui(char* dev_eui)
{
    uint8_t boardId[8];

    BoardGetUniqueId(boardId);

    sprintf(
        dev_eui, "%02X%02X%02X%02X%02X%02X%02X%02X",
        boardId[0], boardId[1], boardId[2], boardId[3],
        boardId[4], boardId[5], boardId[6], boardId[7]
    );

    return dev_eui;
}

int lorawan_init(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region)
{
    EepromMcuInit();

    RtcInit();
    SpiInit(
        &SX1276.Spi,
        (SpiId_t)((sx1276_settings->spi.inst == spi0) ? 0 : 1),
        sx1276_settings->spi.mosi /*MOSI*/,
        sx1276_settings->spi.miso /*MISO*/,
        sx1276_settings->spi.sck /*SCK*/, 
        NC
    );

    SX1276.Spi.Nss.pin = sx1276_settings->spi.nss;
    SX1276.Reset.pin = sx1276_settings->reset;
    SX1276.DIO0.pin = sx1276_settings->dio0;
    SX1276.DIO1.pin = sx1276_settings->dio1;

    SX1276IoInit();

    // check version register
    if (SX1276Read(REG_LR_VERSION) != 0x12) {
        return -1;
    }

    LmHandlerParams.Region = region;

    if ( LmHandlerInit( &LmHandlerCallbacks, &LmHandlerParams ) != LORAMAC_HANDLER_SUCCESS )
    {
        return -1;
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError( 20 );

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams );

    return 0;
}

int lorawan_init_abp(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region, const struct lorawan_abp_settings* abp_settings)
{
    AbpSettings = abp_settings;
    OtaaSettings = NULL;

    return lorawan_init(sx1276_settings, region);
}

int lorawan_init_otaa(const struct lorawan_sx1276_settings* sx1276_settings, LoRaMacRegion_t region, const struct lorawan_otaa_settings* otaa_settings)
{
    AbpSettings = NULL;
    OtaaSettings = otaa_settings;

    return lorawan_init(sx1276_settings, region);
}

int lorawan_join()
{
    LmHandlerJoin( );

    return 0;
}

int lorawan_is_joined()
{
    return (LmHandlerJoinStatus() == LORAMAC_HANDLER_SET);
}

int lorawan_process()
{
    int sleep = 0;

    // Processes the LoRaMac events
    LmHandlerProcess( );

    CRITICAL_SECTION_BEGIN( );
    if( IsMacProcessPending == 1 )
    {
        // Clear flag and prevent MCU to go into low power modes.
        IsMacProcessPending = 0;
    }
    else
    {
        // The MCU wakes up through events
        sleep = 1;
    }
    CRITICAL_SECTION_END( );

    return sleep;
}

int lorawan_process_timeout_ms(uint32_t timeout_ms)
{
    absolute_time_t timeout_time = make_timeout_time_ms(timeout_ms);

    bool joined = lorawan_is_joined();
    
    do {
        lorawan_process();

        if (AppRxData.Port) {
            return 0;
        } else if (joined != lorawan_is_joined()) {
            return 0;
        }
    } while (!best_effort_wfe_or_timeout(timeout_time));
    
    return 1; // timed out
}

int lorawan_send_unconfirmed(const void* data, uint8_t data_len, uint8_t app_port)
{
    LmHandlerAppData_t appData;

    appData.Port = app_port;
    appData.BufferSize = data_len;
    appData.Buffer = (uint8_t*)data;

    if (LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG) != LORAMAC_HANDLER_SUCCESS) {
        return -1;
    }

    return 0;
}

int lorawan_receive(void* data, uint8_t data_len, uint8_t* app_port)
{
    *app_port = AppRxData.Port;
    if (*app_port == 0) {
        return -1;
    }

    int receive_length = AppRxData.BufferSize;

    if (data_len < receive_length) {
        receive_length = data_len;
    }

    memcpy(data, AppRxData.Buffer, receive_length);
    AppRxData.Port = 0;

    return receive_length;
}

void lorawan_debug(bool debug)
{
    Debug = debug;
}

int lorawan_erase_nvm()
{
    if (!NvmDataMgmtFactoryReset()) {
        return -1;
    }

    EepromMcuFlush();

    return 0;
}

static void OnMacProcessNotify( void )
{
    IsMacProcessPending = 1;
}

static void OnNvmDataChange( LmHandlerNvmContextStates_t state, uint16_t size )
{
    if (Debug) {
        DisplayNvmDataChange( state, size );
    }

    EepromMcuFlush();
}

static void OnNetworkParametersChange( CommissioningParams_t* params )
{
    MibRequestConfirm_t mibReq;

    const char* device_eui = NULL;
    const char* app_eui = NULL;
    const char* device_address = NULL;
    const char* app_key = NULL;
    const char* app_session_key = NULL;
    const char* network_session_key = NULL;
    const char* channel_mask = NULL;

    if (OtaaSettings != NULL) {
        params->IsOtaaActivation = 1;

        device_eui = OtaaSettings->device_eui;
        app_eui = OtaaSettings->app_eui;
        app_key = OtaaSettings->app_key;
        channel_mask = OtaaSettings->channel_mask;
    }

    if (AbpSettings != NULL) {
        params->IsOtaaActivation = 0;

        device_address = AbpSettings->device_address;
        app_session_key = AbpSettings->app_session_key;
        network_session_key = AbpSettings->network_session_key;
        channel_mask = AbpSettings->channel_mask;

        // Tell the MAC layer which network server version are we connecting too.
        mibReq.Type = MIB_ABP_LORAWAN_VERSION;
        mibReq.Param.AbpLrWanVersion.Value = ABP_ACTIVATION_LRWAN_VERSION;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NET_ID;
        mibReq.Param.NetID = LORAWAN_NETWORK_ID;
        LoRaMacMibSetRequestConfirm( &mibReq );

        if (device_address != NULL) {
            params->DevAddr = 0;

            for (int i = 0; i < 4; i++) {
                int b;

                sscanf(device_address + i * 2, "%2hhx", &b);

                params->DevAddr = (params->DevAddr << 8) | b;
            }
        } else {
            // Random seed initialization
            srand1( LmHandlerCallbacks.GetRandomSeed( ) );
            // Choose a random device address
            params->DevAddr = randr( 0, 0x01FFFFFF );
        }

        mibReq.Type = MIB_DEV_ADDR;
        mibReq.Param.DevAddr = params->DevAddr;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    if (device_eui != NULL) {
        uint8_t deviceEui[8];

        for (int i = 0; i < 8; i++) {
            int b;

            sscanf(device_eui + i * 2, "%2x", &b);

            deviceEui[i] = b;
        }

        mibReq.Type = MIB_DEV_EUI;
        mibReq.Param.DevEui = deviceEui;
        LoRaMacMibSetRequestConfirm( &mibReq );
        memcpy1( params->DevEui, mibReq.Param.DevEui, 8 );
    }

    if (app_eui != NULL) {
        uint8_t joinEui[8];

        for (int i = 0; i < 8; i++) {
            int b;

            sscanf(app_eui + i * 2, "%2x", &b);

            joinEui[i] = b;
        }

        mibReq.Type = MIB_JOIN_EUI;
        mibReq.Param.JoinEui = joinEui;
        LoRaMacMibSetRequestConfirm( &mibReq );
        memcpy1( params->JoinEui, mibReq.Param.JoinEui, 8 );
    }

    if (app_key) {
        uint8_t appKey[16];

        for (int i = 0; i < 16; i++) {
            int b;

            sscanf(app_key + i * 2, "%2x", &b);

            appKey[i] = b;
        }

        mibReq.Type = MIB_APP_KEY;
        mibReq.Param.AppKey = appKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NWK_KEY;
        mibReq.Param.NwkKey = appKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    if (app_session_key) {
        uint8_t appSessionKey[16];

        for (int i = 0; i < 16; i++) {
            int b;

            sscanf(app_session_key + i * 2, "%2x", &b);

            appSessionKey[i] = b;
        }

        mibReq.Type = MIB_APP_S_KEY;
        mibReq.Param.AppSKey = appSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    if (network_session_key) {
        uint8_t networkSessionKey[16];

        for (int i = 0; i < 16; i++) {
            int b;

            sscanf(network_session_key + i * 2, "%2x", &b);

            networkSessionKey[i] = b;
        }

        mibReq.Type = MIB_F_NWK_S_INT_KEY;
        mibReq.Param.FNwkSIntKey = networkSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_S_NWK_S_INT_KEY;
        mibReq.Param.SNwkSIntKey = networkSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NWK_S_ENC_KEY;
        mibReq.Param.NwkSEncKey = networkSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    if (channel_mask != NULL) {
        uint16_t channelMask[6];

        for (int i = 0; i < 6; i++) {
            int b[2];

            sscanf(channel_mask + i * 4 + 0, "%2x", &b[0]);
            sscanf(channel_mask + i * 4 + 2, "%2x", &b[1]);

            channelMask[i] = (b[0] << 8) | b[1];
        }

        mibReq.Type = MIB_CHANNELS_MASK;
        mibReq.Param.ChannelsMask = channelMask;
        LoRaMacMibSetRequestConfirm( &mibReq );
        
        mibReq.Type = MIB_CHANNELS_DEFAULT_MASK;
        mibReq.Param.ChannelsDefaultMask = channelMask;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    if (Debug) {
        DisplayNetworkParametersUpdate( params );
    }
}

static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn )
{
    if (Debug) {
        DisplayMacMcpsRequestUpdate( status, mcpsReq, nextTxIn );
    }
}

static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn )
{
    if (Debug) {
        DisplayMacMlmeRequestUpdate( status, mlmeReq, nextTxIn );
    }
}

static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
    if (Debug) {
        DisplayJoinRequestUpdate( params );
    }

    if( params->Status == LORAMAC_HANDLER_ERROR )
    {
        LmHandlerJoin( );
    }
    else
    {
        LmHandlerRequestClass( LORAWAN_DEFAULT_CLASS );
    }
}

static void OnTxData( LmHandlerTxParams_t* params )
{
    if (Debug) {
        DisplayTxUpdate( params );
    }
}

static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
    if (Debug) {
        DisplayRxUpdate( appData, params );
    }

    memcpy(AppRxData.Buffer, appData->Buffer, appData->BufferSize);
    AppRxData.BufferSize = appData->BufferSize;
    AppRxData.Port = appData->Port;
}

static void OnClassChange( DeviceClass_t deviceClass )
{
    if (Debug) {
        DisplayClassUpdate( deviceClass );
    }

    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
    {
        .Buffer = NULL,
        .BufferSize = 0,
        .Port = 0,
    };
    LmHandlerSend( &appData, LORAMAC_HANDLER_UNCONFIRMED_MSG );
}

static void OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t* params )
{
    switch( params->State )
    {
        case LORAMAC_HANDLER_BEACON_RX:
        {
            break;
        }
        case LORAMAC_HANDLER_BEACON_LOST:
        case LORAMAC_HANDLER_BEACON_NRX:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    if (Debug) {
        DisplayBeaconUpdate( params );
    }
}

#if( LMH_SYS_TIME_UPDATE_NEW_API == 1 )
static void OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection )
{

}
#else
static void OnSysTimeUpdate( void )
{

}
#endif

static void OnTxPeriodicityChanged( uint32_t periodicity )
{
    TxPeriodicity = periodicity;
}

static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed )
{
    LmHandlerParams.IsTxConfirmed = isTxConfirmed;
}

static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity )
{
    LmHandlerParams.PingSlotPeriodicity = pingSlotPeriodicity;
}
