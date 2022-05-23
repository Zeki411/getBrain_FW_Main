#ifndef __ADS1299_DEF_H__
#define __ADS1299_DEF_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

#define ADS1299_SCALE_UV (0.00002235)
#define ADS1299_SCALE_MV (ADS1299_SCALE_UV*1000)

/* Return Type Definition */
#define ADS1299_OK     (0U)
#define ADS1299_NOT_OK (1U)
typedef uint8_t ADS1299_ReturnType;   

#define ADS1299_SET   (1U)
#define ADS1299_RESET (0U)
typedef uint8_t ADS1299_SetStateType;

/* ADS1299 Default Information */
#define ADS1299_NUMBER_OF_CHANNEL   (8U)
#define ADS1299_NUMBER_OF_REGS      (24U)
#define ADS1299_BYTEDATA_RESOLUTION (3U)
#define ADS1299_DEFAULT_ID          0x3EU


/* ADS1299 Register Address Definition */
#define ADS1299_REG_ID            0x00U
#define ADS1299_REG_CONFIG1       0x01U
#define ADS1299_REG_CONFIG2       0x02U
#define ADS1299_REG_CONFIG3       0x03U
#define ADS1299_REG_LOFF          0x04U
#define ADS1299_REG_CH1SET        0x05U
#define ADS1299_REG_CH2SET        0x06U
#define ADS1299_REG_CH3SET        0x07U
#define ADS1299_REG_CH4SET        0x08U
#define ADS1299_REG_CH5SET        0x09U
#define ADS1299_REG_CH6SET        0x0AU
#define ADS1299_REG_CH7SET        0x0BU
#define ADS1299_REG_CH8SET        0x0CU
#define ADS1299_REG_BIAS_SENSP    0x0DU
#define ADS1299_REG_BIAS_SENSN    0x0EU
#define ADS1299_REG_LOFF_SENSP    0x0FU
#define ADS1299_REG_LOFF_SENSN    0x10U
#define ADS1299_REG_LOFF_FLIP     0x11U
#define ADS1299_REG_LOFF_STATP    0x12U
#define ADS1299_REG_LOFF_STATN    0x13U
#define ADS1299_REG_GPIO          0x14U
#define ADS1299_REG_MISC1         0x15U
#define ADS1299_REG_MISC2         0x16U
#define ADS1299_REG_CONFIG4       0x17U

/* ADS1299 Command Definition */
#define ADS1299_CMD_WAKEUP  0x02U // Wake-up from standby mode
#define ADS1299_CMD_STANDBY 0x04U // Enter Standby mode
#define ADS1299_CMD_RESET   0x06U // Reset the device registers to default
#define ADS1299_CMD_START   0x08U // Start and restart (synchronize) conversions
#define ADS1299_CMD_STOP    0x0AU // Stop conversion
#define ADS1299_CMD_RDATAC  0x10U // Enable Read Data Continuous mode (default mode at power-up)
#define ADS1299_CMD_SDATAC  0x11U // Stop Read Data Continuous mode
#define ADS1299_CMD_RDATA   0x12U // Read data by command supports multiple read back

/* ADS1299 Channel Gain Choices */
#define ADS1299_GAIN01 (0x00U)
#define ADS1299_GAIN02 (0x01U)
#define ADS1299_GAIN04 (0x02U)
#define ADS1299_GAIN06 (0x03U)
#define ADS1299_GAIN08 (0x04U)
#define ADS1299_GAIN12 (0x05U)
#define ADS1299_GAIN24 (0x06U)


/* ADS1299 Channel Input Choices */
#define ADS1299_INPUT_NORMAL     (0x00U)
#define ADS1299_INPUT_SHORTED    (0x01U)
#define ADS1299_INPUT_BIAS_MEAS  (0x02U)
#define ADS1299_INPUT_MVDD       (0x03U)
#define ADS1299_INPUT_TEMP       (0x04U)
#define ADS1299_INPUT_TESTSIG    (0x05U)
#define ADS1299_INPUT_BIAS_DRP   (0x06U)
#define ADS1299_INPUT_BIAS_DRN   (0x07U)

/* ADS1299 Output Data Rate Choices */
#define ADS1299_DR_16K_SPS   (0x00U)
#define ADS1299_DR_8K_SPS    (0x01U)
#define ADS1299_DR_4K_SPS    (0x02U)
#define ADS1299_DR_2K_SPS    (0x03U)
#define ADS1299_DR_1K_SPS    (0x04U)
#define ADS1299_DR_500_SPS   (0x05U)
#define ADS1299_DR_250_SPS   (0x06U)

// #ifdef __cplusplus
// }
// #endif

#endif /* __ADS1299_DEF_H__ */