#ifndef __ADS1299_CFG_H__
#define __ADS1299_CFG_H__

#include <mbed.h>
// #include "GetBrainPlatform_Cfg.h"

// #ifdef __cplusplus
// extern "C" {
// #endif
#define GETBRAIN_PLATFORM_NRF52840

typedef struct {
    void (*Spi_TransmitReceive)(uint8_t *, uint8_t *, uint16_t);
    void (*IoCs_Write)(uint8_t board, uint8_t level);
    void (*IoStart_Write)(uint8_t level);
    void (*IoReset_Write)(uint8_t level);
    void (*IoPwdn_Write)(uint8_t level);
    uint8_t (*IoDrdy_Read)();
    void (*Delay_Ms)(uint8_t ms);
} ADS1299_HwConfigType;

#ifdef GETBRAIN_PLATFORM_NRF52840
#define ADS1299_O_MOSI    P1_1
#define ADS1299_O_MISO    P1_8
#define ADS1299_O_CLK     P0_13
#define ADS1299_O_CS      P1_2
#define ADS1299_O_RST     P0_27
#define ADS1299_O_START   P1_12
#define ADS1299_O_PWDN    P1_11
#define ADS1299_I_DRDY    P0_30
#endif

// #ifdef GETBRAIN_PLATFORM_NRF52840
// #define ADS1299_O_MOSI    P0_13
// #define ADS1299_O_MISO    P0_15
// #define ADS1299_O_CLK     P0_14
// #define ADS1299_O_CS      P0_8
// #define ADS1299_O_RST     P0_5
// #define ADS1299_O_START   P0_4
// #define ADS1299_O_PWDN    P1_9
// #define ADS1299_I_DRDY    P0_6
// #endif

#ifdef GETBRAIN_PLATFORM_PORTENTA_H7
#define ADS1299_O_MOSI    PC_3
#define ADS1299_O_MISO    PC_2
#define ADS1299_O_CLK     PI_1
#define ADS1299_O_CS      PI_0
#define ADS1299_O_RST     PC_15
#define ADS1299_O_START   PD_4
#define ADS1299_O_PWDN    PC_13
#define ADS1299_I_DRDY    PD_5
#endif


/* === Define HW Option for ADS1299 === */
#define ADS1299_USING_HW_RESET
#define ADS1299_USING_HW_START_STOP
#define ADS1299_NUMBER_OF_USED_BOARD      (1U)

#define ADS1299_IO_HIGH HIGH
#define ADS1299_IO_LOW  LOW

/* === Define HW Interface for ADS1299 === */
#define ADS1299_DEBUG_PRINT(STR) \
    Serial.print(STR)

/* HW Specific Function */
extern void ADS1299_Platform_Spi_Init(void);
extern void ADS1299_Platform_IoCs_Write(uint8_t board, uint8_t level);

extern ADS1299_HwConfigType ADS1299_Hw;
extern mbed::SPI ADS1299_HwSpi;

// #ifdef __cplusplus
// }
// #endif


#endif
