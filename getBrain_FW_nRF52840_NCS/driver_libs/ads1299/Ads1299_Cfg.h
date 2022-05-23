#ifndef __ADS1299_CFG_H__
#define __ADS1299_CFG_H__

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <soc.h>

#include <device.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>

#include <nrfx_gpiote.h>
#include <nrfx_spi.h>


#ifdef __cplusplus
extern "C" {
#endif

#include "../circular_buffer/CircularBuffer.h"

#ifdef GETBRAIN_PLATFORM_NRF52840_NCS
#endif

#ifdef GETBRAIN_PLATFORM_NRF52840
#define ADS1299_O_MOSI    P1_1
#define ADS1299_O_MISO    P1_8
#define ADS1299_O_CLK     P0_13
#define ADS1299_O_CS      P1_2
#define ADS1299_O_RST     P0_27
#define ADS1299_O_START   P1_12
#define ADS1299_O_PWDN    P0_9
#define ADS1299_I_DRDY    P1_14
#endif

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

/* Option for SPI HW*/
#define ADS1299_CONFIG_1
#define ADS1299_CONFIG_2
#define ADS1299_CONFIG_3

/* */
typedef struct {
    void (*Spi_TransmitReceive)(uint8_t *, uint16_t, uint8_t *, uint16_t, uint8_t);
    void (*Spi_TransmitReceiveDMA)(uint8_t *, uint16_t, uint8_t *, uint16_t);
    void (*IoCs_Write)(uint8_t board, uint8_t level);
    void (*IoStart_Write)(uint8_t level);
    void (*IoReset_Write)(uint8_t level);
    void (*IoPwdn_Write)(uint8_t level);
    uint8_t (*IoDrdy_Read)();
    void (*Delay_Ms)(uint16_t ms);
} ADS1299_HwConfigType;

/* === Define HW Option for ADS1299 === */
#define ADS1299_USING_HW_RESET
#define ADS1299_USING_HW_START_STOP
#define ADS1299_NUMBER_OF_USED_BOARD      (1U)

#define ADS1299_IO_HIGH GPIO_ACTIVE_HIGH
#define ADS1299_IO_LOW  GPIO_ACTIVE_LOW

/* === Define HW Interface for ADS1299 === */
#define ADS1299_DEBUG_PRINT(STR) \
    Serial.print(STR)

/* HW Specific Function */
extern void ADS1299_Platform_Hw_ConfigGpio(void);
extern void ADS1299_Platform_Hw_ConfigDrdy(uint8_t drdy_cb_mode);
extern void ADS1299_Platform_Hw_ConfigSpi(uint8_t spi_mode);
extern void ADS1299_Platform_Hw_ConfigDrdy_UsingPPI(void);

extern ADS1299_HwConfigType ADS1299_Hw;
extern volatile CircularBuffer_t ADS1299_Platform_EegCirBuf;

#ifdef __cplusplus
}
#endif


#endif
