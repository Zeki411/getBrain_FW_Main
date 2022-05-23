# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(getBrain_FW_nRF5340_NCS)

# NORDIC SDK APP START

target_sources(app PRIVATE 
        src/main.c
        getBrain_BleService/getBrain_BleService.c
        driver_libs/ads1299/Ads1299.c
        driver_libs/ads1299/Ads1299_Cfg.c
        driver_libs/circular_buffer/CircularBuffer.c
        getBrain_Apps/App_CollectStream.c  
)

# NORDIC SDK APP END
zephyr_library_include_directories(.)
