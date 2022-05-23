#include "Ads1299.h"

#define ADS1299_READ_ACCESS  0x20U
#define ADS1299_WRITE_ACCESS 0x40U

/* === Global Variable Definition === */
ADS1299_GlobalConfigType ADS1299_GlbCfg;
uint8_t ADS1299_EEGRawDataBuffer[ADS1299_NUMBER_OF_USED_BOARD][3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION];

/* === Static Variable Definition === */
static ADS1299_GlobalConfigType *ADS1299_pGlbCfg = NULL;

/* === Static Function Definition === */
static ADS1299_ReturnType ADS1299_WriteRegs(uint8_t board, uint8_t address, uint8_t *pData, uint16_t size);
static ADS1299_ReturnType ADS1299_ReadRegs(uint8_t board, uint8_t address, uint8_t *pData, uint16_t size);
static ADS1299_ReturnType ADS1299_SendCmd(uint8_t board, uint8_t cmd);

/* === Static Function Implementation === */
static ADS1299_ReturnType ADS1299_WriteRegs(uint8_t board, uint8_t address, uint8_t *pData, uint16_t size)
{
	uint8_t au8TxData[25];
	uint8_t au8RxData[25];

	/* Check input parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

    if(ADS1299_REG_CONFIG4 < address) 
	{
        return ADS1299_NOT_OK;
    }

    if (0 == size) 
	{
        return ADS1299_NOT_OK;
    }

    /* Prepare Data */
    memset(au8TxData, 0, 25);
    memset(au8RxData, 0, 25);
    au8TxData[0] = (ADS1299_WRITE_ACCESS | (address & 0x1F));
    au8TxData[1] = (size & 0x1F);
    for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
    {
    	au8TxData[2 + u8Cnt] = pData[u8Cnt];
    }

	/* I/O transaction */
    ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_LOW);

    ADS1299_pGlbCfg->pHw->Spi_TransmitReceive(&au8TxData[0], size+2, &au8RxData[0], size+2, ADS1299_RESET);

    ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_HIGH);

	return ADS1299_OK;
}

static ADS1299_ReturnType ADS1299_ReadRegs(uint8_t board, uint8_t address, uint8_t *pData, uint16_t size)
{
	uint8_t au8TxData[25];
	uint8_t au8RxData[25];

	/* Check input parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

    if(address > ADS1299_REG_CONFIG4) {
        return ADS1299_NOT_OK;
    }
    if (size == 0) {
        return ADS1299_NOT_OK;
    }

    /* Prepare Data */
    memset(au8TxData, 0, 25);
    memset(au8RxData, 0, 25);
    au8TxData[0] = (ADS1299_READ_ACCESS | (address & 0x1F));
    au8TxData[1] = (size & 0x1F);
    for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
    {
    	au8TxData[2 + u8Cnt] = 0;
    }

	/* I/O transaction */
    ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_LOW);

    ADS1299_pGlbCfg->pHw->Spi_TransmitReceive(au8TxData, size+2, au8RxData, size+2, ADS1299_RESET);

    ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_HIGH);

    for(uint8_t u8Cnt = 0; u8Cnt < size; u8Cnt++)
    {
    	pData[u8Cnt] = au8RxData[2 + u8Cnt];
    }

	return ADS1299_OK;
}

static ADS1299_ReturnType ADS1299_SendCmd(uint8_t board, uint8_t cmd)
{
	uint8_t u8TxData = 0;
	uint8_t u8RxData = 0;

	/* Check input parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* Prepare Data */
	u8TxData = cmd;

	/* I/O transaction */
	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_LOW);

	ADS1299_pGlbCfg->pHw->Spi_TransmitReceive(&u8TxData, 1, &u8RxData, 1, ADS1299_RESET);

	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_HIGH);

	return ADS1299_OK;
}

/* === Driver Function Implementation === */
void ADS1299_CmdRESET(void)
{
#ifdef ADS1299_USING_HW_RESET
	ADS1299_pGlbCfg->pHw->IoReset_Write(ADS1299_IO_LOW);
	ADS1299_pGlbCfg->pHw->Delay_Ms(1);
	ADS1299_pGlbCfg->pHw->IoReset_Write(ADS1299_IO_HIGH);

#else
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_RESET);
	}
#endif

	ADS1299_pGlbCfg->pHw->Delay_Ms(1); // Recommended to wait at least 18 Tclk before using device (~8uS);
}

void ADS1299_CmdSTART(void)
{
#ifdef ADS1299_USING_HW_START_STOP
	ADS1299_pGlbCfg->pHw->IoStart_Write(ADS1299_IO_HIGH);
#else
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_START);
	}
#endif
}

void ADS1299_CmdSTOP(void)
{
#ifdef ADS1299_USING_HW_START_STOP
	ADS1299_pGlbCfg->pHw->IoStart_Write(ADS1299_IO_LOW);
#else
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_STOP);
	}
#endif
}

void ADS1299_CmdWAKEUP(void)
{
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_WAKEUP);
	}
	ADS1299_pGlbCfg->pHw->Delay_Ms(1);
}

void ADS1299_CmdSDATAC(void)
{
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_SDATAC);
	}
	ADS1299_pGlbCfg->pHw->Delay_Ms(1);
}

void ADS1299_CmdRDATAC(void)
{
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_SendCmd(u8BoardCnt, ADS1299_CMD_RDATAC);
	}
	ADS1299_pGlbCfg->pHw->Delay_Ms(1);
}

uint8_t ADS1299_GetID(uint8_t board)
{
	uint8_t u8Id = 0;

	ADS1299_ReadRegs(board, ADS1299_REG_ID, &u8Id, 1);

	return u8Id;
}

ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg1(uint8_t board, \
                                                   ADS1299_SetStateType ndaisy_en, \
												   ADS1299_SetStateType clk_en, \
												   uint8_t data_rate)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* Setup Data */
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG1.opt.clk_en = clk_en;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG1.opt.ndaisy_en = ndaisy_en;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG1.opt.dr = data_rate;

	/* Config CONFIG1 */
	ADS1299_WriteRegs(board, ADS1299_REG_CONFIG1, &ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG1.value, 1);

	return ADS1299_OK;
}


ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg2(uint8_t board, \
                                                   ADS1299_SetStateType int_cal, \
												   ADS1299_SetStateType cal_amp, \
												   ADS1299_SetStateType cal_freq)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* Setup Data */
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG2.opt.int_cal = int_cal;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG2.opt.cal_amp = cal_amp;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG2.opt.cal_freq = cal_freq;

	/* Config CONFIG2 */
	ADS1299_WriteRegs(board, ADS1299_REG_CONFIG2, &ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG2.value, 1);

	return ADS1299_OK;
}


ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg3(uint8_t board, \
                                                   ADS1299_SetStateType npd_refbuf, \
                                                   ADS1299_SetStateType bias_meas, \
												   ADS1299_SetStateType biasref_int, 
												   ADS1299_SetStateType npd_bias,
												   ADS1299_SetStateType bias_loff_sens, \
												   ADS1299_SetStateType bias_stat)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* Setup Data */
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.npd_refbuf = npd_refbuf;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.bias_meas = bias_meas;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.biasref_int = biasref_int;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.npd_bias = npd_bias;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.bias_loff_sens = bias_loff_sens;
	ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.opt.bias_stat = bias_stat;

	/* Config CONFIG3 */
	ADS1299_WriteRegs(board, ADS1299_REG_CONFIG3, &ADS1299_pGlbCfg->RegTable[board].GlbChnRegs.CONFIG3.value, 1);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_ConfigChannelSettings(uint8_t board, uint8_t channel, ADS1299_SetStateType pwdn, uint8_t gain, uint8_t input_type, ADS1299_SetStateType srb2)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	if(ADS1299_NUMBER_OF_CHANNEL <= channel)
	{
		return ADS1299_NOT_OK;
	}

	/* Setup Data */
	ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[channel].opt.pd = pwdn;
	ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[channel].opt.gain = gain;
	ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[channel].opt.srb2 = srb2;
	ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[channel].opt.mux = input_type;

	/* Config Channel Control Settings */
	ADS1299_WriteRegs(board, ADS1299_REG_CH1SET + channel, &ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[channel].value, 1);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_ConfigAllChannelSettings(uint8_t board, ADS1299_SetStateType pwdn, uint8_t gain, uint8_t input_type, ADS1299_SetStateType srb2)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* Setup Data */
	for(uint8_t u8Cnt = 0; u8Cnt < ADS1299_NUMBER_OF_CHANNEL; u8Cnt++)
	{
		ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[u8Cnt].opt.pd = pwdn;
	    ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[u8Cnt].opt.gain = gain;
	    ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[u8Cnt].opt.srb2 = srb2;
	    ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[u8Cnt].opt.mux = input_type;
	}
	/* Config Channel Control Settings */
	ADS1299_WriteRegs(board, ADS1299_REG_CH1SET, &(ADS1299_pGlbCfg->RegTable[board].ChnCtrlRegs.CHSET[0].value), ADS1299_NUMBER_OF_CHANNEL);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_ConfigBiasSensP(uint8_t board, \
	                                       uint8_t biasp1_en, \
                                           uint8_t biasp2_en, \
										   uint8_t biasp3_en, \
										   uint8_t biasp4_en, \
										   uint8_t biasp5_en, \
										   uint8_t biasp6_en, \
										   uint8_t biasp7_en, \
										   uint8_t biasp8_en)
{

	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	// Config Bias Sens Positive
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx1 = biasp1_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx2 = biasp2_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx3 = biasp3_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx4 = biasp4_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx5 = biasp5_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx6 = biasp6_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx7 = biasp7_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].opt.biasx8 = biasp8_en;

	ADS1299_WriteRegs(board, ADS1299_REG_BIAS_SENSP, &(ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[0].value), 1);
	
	return ADS1299_OK;
}


ADS1299_ReturnType ADS1299_ConfigBiasSensN(uint8_t board, \
	                                       uint8_t biasn1_en, \
                                           uint8_t biasn2_en, \
										   uint8_t biasn3_en, \
										   uint8_t biasn4_en, \
										   uint8_t biasn5_en, \
										   uint8_t biasn6_en, \
										   uint8_t biasn7_en, \
										   uint8_t biasn8_en)
{

	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	// Config Bias Sens Negative
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx1 = biasn1_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx2 = biasn2_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx3 = biasn3_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx4 = biasn4_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx5 = biasn5_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx6 = biasn6_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx7 = biasn7_en;
	ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].opt.biasx8 = biasn8_en;

	ADS1299_WriteRegs(board, ADS1299_REG_BIAS_SENSN, &(ADS1299_pGlbCfg->RegTable[board].LeadOffDetectCDCtrlRegs.BIAS_SENS[1].value), 1);
	
	return ADS1299_OK;
}


ADS1299_ReturnType ADS1299_GetAllRegsValue(uint8_t board)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	ADS1299_ReadRegs(board, ADS1299_REG_ID, (uint8_t *)&(ADS1299_pGlbCfg->RegTable[board]), ADS1299_NUMBER_OF_REGS);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_SetSrb1(uint8_t board, ADS1299_SetStateType srb1)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	ADS1299_pGlbCfg->RegTable[board].GpioMiscRegs.MISC1.opt.srb1 = srb1;

	ADS1299_WriteRegs(board, ADS1299_REG_MISC1, &ADS1299_pGlbCfg->RegTable[board].GpioMiscRegs.MISC1.value, 1);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_GetSensorData(uint8_t board)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* I/O Transaction */
	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_LOW);

	ADS1299_pGlbCfg->pHw->Spi_TransmitReceive(NULL, \
	                                          0, \
	                                          &ADS1299_EEGRawDataBuffer[board][0], \
	                                          3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION, ADS1299_SET);

	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_HIGH);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_GetSensorDataWithDMA(uint8_t board)
{
	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* I/O Transaction */

	ADS1299_pGlbCfg->pHw->Spi_TransmitReceiveDMA(NULL, \
	                                          0, \
	                                          &ADS1299_EEGRawDataBuffer[board][0], \
	                                          3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_Poll2GetSensorData(uint8_t board)
{
	/* Check Data ready */
	if(ADS1299_pGlbCfg->pHw->IoDrdy_Read() != ADS1299_IO_LOW)
	{
		return ADS1299_NOT_OK;
	}

	/* Check Parameters */
	if(ADS1299_NUMBER_OF_USED_BOARD <= board)
	{
		return ADS1299_NOT_OK;
	}

	/* I/O Transaction */
	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_LOW);

	ADS1299_pGlbCfg->pHw->Spi_TransmitReceive(NULL, \
	                                          0, \
	                                          &ADS1299_EEGRawDataBuffer[board][0], \
	                                          3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION, ADS1299_SET);

	ADS1299_pGlbCfg->pHw->IoCs_Write(board, ADS1299_IO_HIGH);

	return ADS1299_OK;
}

ADS1299_ReturnType ADS1299_Init(void)
{
	/* Assign Global Config Pointer */
	ADS1299_pGlbCfg = &ADS1299_GlbCfg;
	ADS1299_pGlbCfg->pHw = &ADS1299_Hw; //Assign Hw Attributes

	ADS1299_CmdRESET(); //Reset HW
	ADS1299_CmdSDATAC(); //Stop DataContinuousMode

	/* Start Initialization */
	ADS1299_pGlbCfg->pHw->IoPwdn_Write(ADS1299_IO_HIGH);
	ADS1299_pGlbCfg->pHw->IoReset_Write(ADS1299_IO_HIGH);
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_pGlbCfg->pHw->IoCs_Write(u8BoardCnt, ADS1299_IO_HIGH);
	}

	ADS1299_pGlbCfg->pHw->Delay_Ms(1000);

	ADS1299_CmdRESET(); //Reset HW
	ADS1299_pGlbCfg->pHw->Delay_Ms(1000);

	ADS1299_CmdSDATAC(); //Stop DataContinuousMode
	ADS1299_pGlbCfg->pHw->Delay_Ms(1000);

	/* Config CONFIG2 & CONFIG3 */
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		ADS1299_pGlbCfg->RegTable[u8BoardCnt].GlbChnRegs.CONFIG2.value = 0xD0;
		ADS1299_pGlbCfg->RegTable[u8BoardCnt].GlbChnRegs.CONFIG3.value = 0xE0;
		ADS1299_WriteRegs(u8BoardCnt, ADS1299_REG_CONFIG2, &ADS1299_pGlbCfg->RegTable[u8BoardCnt].GlbChnRegs.CONFIG2.value, 2);
	}

	/* Check Device Information */
	for(uint8_t u8BoardCnt = 0; u8BoardCnt < ADS1299_NUMBER_OF_USED_BOARD; u8BoardCnt++)
	{
		if(ADS1299_DEFAULT_ID != ADS1299_GetID(u8BoardCnt))
		{
			// ADS1299_DEBUG_PRINT("[ADS1299_ERR] Fail to read ID\r\n");
			return ADS1299_NOT_OK;
		}

		/* Update All Register Data */
	    ADS1299_GetAllRegsValue(u8BoardCnt);
	}

	// ADS1299_DEBUG_PRINT("[ADS1299_MSG] Complete Init\r\n");
	return ADS1299_OK;
}


