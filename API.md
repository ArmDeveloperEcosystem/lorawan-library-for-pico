# pico-lorawan API

## Include Library

```c
#include <pico/lorawan.h>
```

## Initialization

### SX1276 Settings

```c
// pin configuration for SX1276 radio module
struct lorawan_sx12xx_settings sx12xx_settings = {
    .spi = {
        .inst = PICO_DEFAULT_SPI_INSTANCE, // RP2040 SPI instance
        .mosi = PICO_DEFAULT_SPI_TX_PIN,   // SPI MOSI GPIO
        .miso = PICO_DEFAULT_SPI_RX_PIN,   // SPI MISO GPIO
        .sck = PICO_DEFAULT_SPI_SCK_PIN,   // SPI SCK GPIO
        .nss = 8.                          // SPI NSS / CS GPIO
    },
    .reset = 9,                            // SX1276 RESET GPIO
    .dio0 = 7,                             // SX1276 DIO0 / G0 GPIO
    .dio1 = 10                             // SX1276 DIO0 / G1 GPIO
};
```

### ABP

Initialize the library for ABP.

```c
// ABP settings
const struct lorawan_abp_settings abp_settings = {
    // LoRaWAN device address (32-bit)
    .device_address = "00000000",
  
    // LoRaWAN Network Session Key (128-bit)
    .network_session_key = "00000000000000000000000000000000",
  
    // LoRaWAN Application Session Key (128-bit)
    .app_session_key = "00000000000000000000000000000000",
  
    // LoRaWAN Channel Mask,  NULL value will use the default channel mask
    // for US915 with TTN use "FF0000000000000000020000"
    .channel_mask = NULL,
};

int lorawan_init_abp(const struct lorawan_sx12xx_settings* sx12xx_settings, LoRaMacRegion_t region, const struct lorawan_abp_settings* abp_settings);
```

- `sx12xx_settings` - pointer to settings for SX1276 SPI and GPIO pins
- `region` - region to use, see [`enum LoRaMacRegion_t
`](http://stackforce.github.io/LoRaMac-doc/LoRaMac-doc-v4.5.1/group___l_o_r_a_m_a_c.html#ga3b9d54f0355b51e85df8b33fd1757eec)for supported values]
- `abp_settings` - pointer to LoRaWAN ABP settings

Returns `0` on success, `-1` on error.

### OTAA

Initialize the library for OTAA.

```c
const struct lorawan_otaa_settings otaa_settings = {
    // LoRaWAN Device EUI (64-bit), NULL value will use Default Dev EUI
    .device_eui   = "0000000000000000",
  
    // LoRaWAN Application / Join EUI (64-bit)
    .app_eui      = "0000000000000000",
  
    // LoRaWAN Application Key (128-bit)
    .app_key      = "00000000000000000000000000000000",

    // LoRaWAN Channel Mask,  NULL value will use the default channel mask
    // for US915 with TTN use "FF0000000000000000020000"
    .channel_mask = NULL,
};

int lorawan_init_otaa(const struct lorawan_sx12xx_settings* sx12xx_settings, LoRaMacRegion_t region, const struct lorawan_otaa_settings* otaa_settings);
```

- `sx12xx_settings` - pointer to settings for SX1276 SPI and GPIO pins
- `region` - region to use, see [`enum LoRaMacRegion_t
`](http://stackforce.github.io/LoRaMac-doc/LoRaMac-doc-v4.5.1/group___l_o_r_a_m_a_c.html#ga3b9d54f0355b51e85df8b33fd1757eec)for supported values]
- `otaa_settings` - pointer to LoRaWAN OTAA settings

Returns `0` on success, `-1` on error.


## Joining

### Start Join

Start the LoRaWAN network join process.

```c
int lorawan_join();
```

Returns `0` on success, `-1` on error.

### Join Status

Query the LoRaWAN network join status.

```c
int lorawan_is_joined();
```

Returns `1` if the board has successfully joined the LoRaWAN network, `0` otherwise.

## Processing Pending Events

### Without Timeout

Let the LoRaWAN library process pending events.

```c
int lorawan_process();
```

Returns `0` if there is a pending event, `1` if there are no pending events and the calling application can go into low power sleep mode.

### With Timeout

Let the lorwan library process pending events for up to `n` milliseconds.

```c
int lorawan_process_timeout_ms(uint32_t timeout_ms);
```

- `timeout_ms` in milliseconds to wait for LoRaWAN event.

Returns `0` on event, `1` on timeout.


## Sending Uplink Messages

### Unconfirmed

Send an unconfirmed uplink message.

```c
int lorawan_send_unconfirmed(const void* data, uint8_t data_len, uint8_t app_port);
```

- `data` - message data buffer to send
- `data_len` - size of message in bytes
- `app_port` - application port to use for message

Returns `0` on success, `-1` on failure.

## Receiving Downlink Messages

```c
int lorawan_receive(void* data, uint8_t data_len, uint8_t* app_port);
```

- `data` - message data buffer to store received data
- `data_len` - size of message data buffer in bytes
- `app_port` - pointer to store application port of received message

Returns length of received message on success, `-1` on failure.

## Other

### Default Dev EUI

Read the board's default Dev EUI Dev EUI which is based on the Pico SDK's [pico_get_unique_board_id(...)](https://raspberrypi.github.io/pico-sdk-doxygen/group__pico__unique__id.html) API which uses the on board NOR flash device 64-bit unique ID.

```c
const char* lorawan_default_dev_eui(char* dev_eui);
```

- `dev_eui` - 17 byte buffer to store default Dev EUI as c-string

Returns `dev_eui` argument.

### Debugging Ouput

Enable or disable debug output from the library.

```c
void lorawan_debug(bool debug);
```

- `debug` - `true` to enable debug output, `false` to disable debug output
