# pico-lorawan
Enable LoRaWAN communications on your [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/) or any RP2040 based board using a [Semtech SX1276 radio module](https://www.semtech.com/apps/product.php?pn=SX1276) or [Waveshare's SX1262-based LoRaWAN module](https://www.waveshare.com/wiki/Pico-LoRa-SX1262-868M).

Based on the Semtech's [LoRaWAN end-device stack implementation and example projects](https://github.com/Lora-net/LoRaMac-node).

## Hardware

 * RP2040 board
   * [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/)
   * [Adafruit Feather RP2040](https://www.adafruit.com/product/4884)
 * Semtech SX1276 board
   * [Adafruit RFM95W LoRa Radio Transceiver Breakout - 868 or 915 MHz - RadioFruit](https://www.adafruit.com/product/3072)
   * [Adafruit LoRa Radio FeatherWing - RFM95W 900 MHz - RadioFruit](https://www.adafruit.com/product/3231) 
 * Semtech SX1262 board
   * [Waveshare Pico-LoRa-SX1262-868M](https://www.waveshare.com/wiki/Pico-LoRa-SX1262-868M) | [Shop](https://www.waveshare.com/pico-lora-sx1262-868m.htm)

### Default Pinout

Default Pinout for Semtech SX1276:

| Raspberry Pi Pico / RP2040 | Semtech SX1276 |
| ----------------- | -------------- |
| 3.3V | VCC |
| GND | GND |
| GPIO 18 | SCK |
| GPIO 19 | MOSI |
| GPIO 16 | MISO |
| GPIO 7 | DIO0 / G0 |
| GPIO 8 | NSS / CS |
| GPIO 9 | RESET |
| GPIO 10 | DIO1 / G1 |

Default Pinout for Semtech SX1262 on Waveshare Pico-LoRa-SX1262-868M:

| Raspberry Pi Pico / RP2040 | Waveshare Pico-LoRa-SX1262-868M |
| ----------------- | -------------- |
| 3.3V | VCC |
| GND | GND |
| GPIO 10 | SCK |
| GPIO 11 | MOSI |
| GPIO 12 | MISO |
| GPIO 3 | NSS / CS |
| GPIO 15 | RESET |
| GPIO 20 | DIO1 |
| GPIO 2 | BUSY |

GPIO pins are configurable in examples or API.

## Enabling support for Waveshare Pico-LoRa-SX1262-868M

By default this library only supports the Semtech SX1276 boards mentioned in the first section. Support for Waveshare's SX1262-based Raspberry Pi Pico Hat is available, but needs to be enabled actively. 

Take the following steps to switch from SX1276 to Waveshare SX1262 support: 

Uncomment 

```
set(WITH_WAVESHARE_SX126X ON)
```

in  `CMakeLists.txt`.

In your example application, e.g. otaa_temperature_led/ set the region to EU868 in `config.h`:

```
#define LORAWAN_REGION          LORAMAC_REGION_EU868
```

_(and also provide your Device EUI, App EUI and App Key here!)_

In the example application's `main.c` make sure that you're using the proper `sx12xx_settings` for your SX1262 Waveshare module:

```
const struct lorawan_sx12xx_settings sx12xx_settings = {
    .spi = {
        .inst = spi1,
        .mosi = 11,
        .miso = 12,
        .sck  = 10,
        .nss  = 3
    },
    .reset = 15,
    .busy = 2,
    .dio1  = 20
};
```

_**There is no guarantee that other SX1262-based boards will work. Tests have only been run using Waveshare's board.**_

## Examples

See [examples](examples/) folder.

There is a `config.h` file to your ABP or OTAA node configuration for each example.

## Cloning

```sh
git clone --recurse-submodules https://github.com/sandeepmistry/pico-lorawan.git 
```

## Building

1. [Set up the Pico C/C++ SDK](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. Set `PICO_SDK_PATH`
```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```
3. Create `build` dir, run `cmake` and `make`:
```
mkdir build
cd build
cmake .. -DPICO_BOARD=pico
make
```
4. Copy example `.uf2` to Pico when in BOOT mode.

## Acknowledgements

A big thanks to [Alasdair Allan](https://github.com/aallan) for his initial testing of EU868 support!

Thanks to Waveshare and [siuwahzhong](https://github.com/siuwahzhong) for providing their modified version of this library. Parts of their work have been merged back into this library by [Thomas Leister](https://github.com/ThomasLeister). Original source: [ZIP](https://www.waveshare.com/w/upload/0/08/Pico-LoRa-SX1262-868M_Code.zip) | [GitHub](https://github.com/siuwahzhong/lorawan-library-for-pico) 

This project was created on behalf of the [Arm Software Developers](https://developer.arm.com/) team, follow them on Twitter: [@ArmSoftwareDev](https://twitter.com/armsoftwaredev) and YouTube: [Arm Software Developers](https://www.youtube.com/channel/UCHUAckhCfRom2EHDGxwhfOg) for more resources!

