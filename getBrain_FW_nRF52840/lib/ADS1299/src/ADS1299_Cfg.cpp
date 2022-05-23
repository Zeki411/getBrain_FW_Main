#include "ADS1299_Cfg.h"

mbed::SPI ADS1299_HwSpi(ADS1299_O_MOSI, ADS1299_O_MISO, ADS1299_O_CLK);

static void ADS1299_Platform_Spi_TxRx(uint8_t *pTxData, uint8_t *pRxData, uint16_t size);
// static void ADS1299_Platform_IoCs_Write(uint8_t board, uint8_t level);
static void ADS1299_Platform_IoStart_Write(uint8_t level);
static void ADS1299_Platform_IoReset_Write(uint8_t level);
static void ADS1299_Platform_IoPwdn_Write(uint8_t level);
static uint8_t ADS1299_Platform_IoDrdy_Read();
static void ADS1299_Platform_Delay(uint8_t ms);

/* === Define HW Interface for ADS1299 === */
ADS1299_HwConfigType ADS1299_Hw = {
    &ADS1299_Platform_Spi_TxRx,
    &ADS1299_Platform_IoCs_Write,
    &ADS1299_Platform_IoStart_Write,
    &ADS1299_Platform_IoReset_Write,
    &ADS1299_Platform_IoPwdn_Write,
    &ADS1299_Platform_IoDrdy_Read,
    &ADS1299_Platform_Delay,
};

static void ADS1299_Platform_Spi_TxRx(uint8_t *pTxData, uint8_t *pRxData, uint16_t size)
{
    if(pTxData == NULL)
    {
        for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
        {
            pRxData[u8Cnt] = ADS1299_HwSpi.write(0x00);
        }  
    }
    else if(pRxData == NULL)
    {
        for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
        {
            ADS1299_HwSpi.write(pTxData[u8Cnt]);
        }
    }
    else
    {
        for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
        {
            pRxData[u8Cnt] = ADS1299_HwSpi.write(pTxData[u8Cnt]);
        }  
    }
}

void ADS1299_Platform_IoCs_Write(uint8_t board, uint8_t level)
{
    if(board == 0)
    {
        digitalWrite(ADS1299_O_CS, (PinStatus)level);
    }
}

static void ADS1299_Platform_IoStart_Write(uint8_t level)
{
    digitalWrite(ADS1299_O_START, (PinStatus)level);
}

static void ADS1299_Platform_IoReset_Write(uint8_t level)
{
    digitalWrite(ADS1299_O_RST, (PinStatus)level);
}

static void ADS1299_Platform_IoPwdn_Write(uint8_t level)
{
    digitalWrite(ADS1299_O_PWDN, (PinStatus)level);
}

static uint8_t ADS1299_Platform_IoDrdy_Read()
{ 
    uint8_t u8State = 0;
    u8State = (uint8_t)digitalRead(ADS1299_I_DRDY);
    return u8State;
}

static void ADS1299_Platform_Delay(uint8_t ms) 
{
    delay(ms);
}

void ADS1299_Platform_Spi_Init(void)
{
    ADS1299_HwSpi.format(8,1);  //CPOL = 0, CPHA = 1
    ADS1299_HwSpi.frequency(8000000);
    // ADS1299_HwSpi.set_dma_usage((DMAUsage)DMA_USAGE_NEVER);
}