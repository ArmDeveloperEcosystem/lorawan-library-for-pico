cmake_minimum_required(VERSION 3.12)

# initialize pico_sdk from GIT
# (note this can come from environment, CMake cache etc)
# set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(pico_lorawan)

# initialize the Pico SDK
pico_sdk_init()

set(LORAMAC_NODE_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/LoRaMac-node)

# Choose whether to enable Waveshare SX1262 module support instead of SX1276 support.
# This will control cmake's build behavior.
option(WITH_WAVESHARE_SX126X "Enable Waveshare SX1262 Raspberry Pi LoRa module support instead of SX1276" OFF)

# Enable for use with SX126X, e.g. Waveshare Raspberry Pi Pico LoRa module SX1262 by uncommenting:
#set(WITH_WAVESHARE_SX126X ON)

## Depending on previous selection, add the following items
if(WITH_WAVESHARE_SX126X)
    message("Building for SX126X")
    set(SX12XX_RADIO_FILES 
        ${LORAMAC_NODE_PATH}/src/radio/sx126x/sx126x.c 
        ${LORAMAC_NODE_PATH}/src/radio/sx126x/radio.c
    )
    set(SX12XX_BOARD_FILES ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/sx126x-board.c)
else()
    message("Building for SX1276")
    set(SX12XX_RADIO_FILES 
        ${LORAMAC_NODE_PATH}/src/radio/sx1276/sx1276.c 
    )
    set(SX12XX_BOARD_FILES ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/sx1276-board.c) 
endif()


add_library(pico_loramac_node INTERFACE)

target_sources(pico_loramac_node INTERFACE
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/CayenneLpp.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/cli.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandlerMsgDisplay.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/NvmDataMgmt.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/LmHandler.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages/FragDecoder.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages/LmhpClockSync.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages/LmhpCompliance.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages/LmhpFragmentation.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages/LmhpRemoteMcastSetup.c
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/NvmDataMgmt.c

    ${LORAMAC_NODE_PATH}/src/boards/mcu/utilities.c

    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/NvmDataMgmt.c

    ${LORAMAC_NODE_PATH}/src/mac/region/Region.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionAS923.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionAU915.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionBaseUS.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN470.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN470A20.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN470A26.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN470B20.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN470B26.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCN779.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionCommon.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionEU433.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionEU868.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionIN865.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionKR920.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionRU864.c
    ${LORAMAC_NODE_PATH}/src/mac/region/RegionUS915.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMac.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacAdr.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacClassB.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacCommands.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacConfirmQueue.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacCrypto.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacParser.c
    ${LORAMAC_NODE_PATH}/src/mac/LoRaMacSerializer.c

    ${LORAMAC_NODE_PATH}/src/peripherals/soft-se/aes.c
    ${LORAMAC_NODE_PATH}/src/peripherals/soft-se/cmac.c
    ${LORAMAC_NODE_PATH}/src/peripherals/soft-se/soft-se-hal.c
    ${LORAMAC_NODE_PATH}/src/peripherals/soft-se/soft-se.c

    ${SX12XX_RADIO_FILES}

    ${LORAMAC_NODE_PATH}/src/system/delay.c
    ${LORAMAC_NODE_PATH}/src/system/gpio.c
    ${LORAMAC_NODE_PATH}/src/system/nvmm.c
    ${LORAMAC_NODE_PATH}/src/system/systime.c
    ${LORAMAC_NODE_PATH}/src/system/timer.c

    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/delay-board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/eeprom-board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/gpio-board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/rtc-board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/boards/rp2040/spi-board.c
    
    ${SX12XX_BOARD_FILES}
)

target_include_directories(pico_loramac_node INTERFACE
    ${LORAMAC_NODE_PATH}/src
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler
    ${LORAMAC_NODE_PATH}/src/apps/LoRaMac/common/LmHandler/packages
    ${LORAMAC_NODE_PATH}/src/boards
    ${LORAMAC_NODE_PATH}/src/mac
    ${LORAMAC_NODE_PATH}/src/mac/region
    ${LORAMAC_NODE_PATH}/src/peripherals/soft-se
    ${LORAMAC_NODE_PATH}/src/radio
    ${LORAMAC_NODE_PATH}/src/system
)

target_link_libraries(pico_loramac_node INTERFACE pico_stdlib pico_unique_id hardware_spi)

target_compile_definitions(pico_loramac_node INTERFACE -DSOFT_SE)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_EU868)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_US915)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_CN779)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_EU433)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_AU915)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_AS923)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_CN470)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_KR920)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_IN865)
target_compile_definitions(pico_loramac_node INTERFACE -DREGION_RU864)
if(WITH_WAVESHARE_SX126X)
    target_compile_definitions(pico_loramac_node INTERFACE -DACTIVE_REGION=LORAMAC_REGION_EU868)
else()
    target_compile_definitions(pico_loramac_node INTERFACE -DACTIVE_REGION=LORAMAC_REGION_US915)
endif()

add_library(pico_lorawan INTERFACE)

target_sources(pico_lorawan INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/lorawan.c
)

target_include_directories(pico_lorawan INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

target_link_libraries(pico_lorawan INTERFACE pico_loramac_node)

# define sx1262 for preprocessor if Waveshare sx1262 support is enabled in cmake.
# Also do not build all examples for now (they are partly incompatible)
if(WITH_WAVESHARE_SX126X)
    add_definitions(-Dsx126x)
    add_subdirectory("examples/otaa_temperature_led")
else()
    add_definitions(-Dsx1276)
    add_subdirectory("examples/default_dev_eui")
    add_subdirectory("examples/hello_abp")
    add_subdirectory("examples/hello_otaa")
    add_subdirectory("examples/otaa_temperature_led")
endif()


