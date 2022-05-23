#ifndef __ADS1299_H__
#define __ADS1299_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "ADS1299_Cfg.h"
#include "ADS1299_Def.h"

typedef uint8_t ADS1299_ReturnType;

typedef struct {
    union {
        struct {
            uint8_t dr        :3;
            uint8_t rsv2      :2;
            uint8_t clk_en    :1;
            uint8_t ndaisy_en :1;
            uint8_t rsv1      :1;
        } opt;
        uint8_t value;
    } CONFIG1;

    union {
        struct {
            uint8_t cal_freq :2;
            uint8_t cal_amp  :1;
            uint8_t rsv2     :1;
            uint8_t int_cal  :1;
            uint8_t rsv1     :3;    
        } opt;
        uint8_t value;
    } CONFIG2;

    union {
        struct {
            uint8_t bias_stat      :1;
            uint8_t bias_loff_sens :1;
            uint8_t npd_bias       :1;
            uint8_t biasref_int    :1;
            uint8_t bias_meas      :1;
            uint8_t rsv1           :2;
            uint8_t npd_refbuf     :1;
        } opt;
        uint8_t value;
    } CONFIG3;

    union {
        struct {
            uint8_t flead_off :2;
            uint8_t ilead_off :2;
            uint8_t rsv1      :1;
            uint8_t compt_th  :3;
        } opt;
        uint8_t value;
    } LOFF;

} ADS1299_GlobalChannelRegType;

typedef struct {
    union {
        struct {
            uint8_t mux  :3;
            uint8_t srb2 :1;
            uint8_t gain :3;
            uint8_t pd   :1;
        } opt;
        uint8_t value;
    } CHSET[ADS1299_NUMBER_OF_CHANNEL];

} ADS1299_ChannelControlRegType;

typedef struct
{
    union {
        struct {
            uint8_t biasx1 :1;
            uint8_t biasx2 :1;
            uint8_t biasx3 :1;
            uint8_t biasx4 :1;
            uint8_t biasx5 :1;
            uint8_t biasx6 :1;
            uint8_t biasx7 :1;
            uint8_t biasx8 :1;
        } opt;
        uint8_t value;  
    } BIAS_SENS[2];

    union {
        struct {
            uint8_t loffx1 :1;
            uint8_t loffx2 :1;
            uint8_t loffx3 :1;
            uint8_t loffx4 :1;
            uint8_t loffx5 :1;
            uint8_t loffx6 :1;
            uint8_t loffx7 :1;
            uint8_t loffx8 :1;
        } opt;
        uint8_t value;  
    } LOFF_SENS[2];

    union {
        struct {
            uint8_t loff_flip1 :1;
            uint8_t loff_flip2 :1;
            uint8_t loff_flip3 :1;
            uint8_t loff_flip4 :1;
            uint8_t loff_flip5 :1;
            uint8_t loff_flip6 :1;
            uint8_t loff_flip7 :1;
            uint8_t loff_flip8 :1;
        } opt;
        uint8_t value;  
    } LOFF_FLIP;

} ADS1299_LeadOffDetectAndCurDirCtrlRegType;

typedef struct
{
    union {
        struct {
            uint8_t gpioc :4;
            uint8_t gpiod :4;
        } opt;
        uint8_t value;
    } GPIO;

    union {
        struct {
            uint8_t srv2 :5;
            uint8_t srb1 :1;
            uint8_t rsv1 :2;
        } opt;
        uint8_t value;
    } MISC1;

    uint8_t MSIC2;

    union {
        struct {
            uint8_t rsv1          :4;
            uint8_t single_shot   :1;
            uint8_t rsv2          :1;
            uint8_t npd_loff_comp :1;
            uint8_t rsv3          :1;
        } opt;
        uint8_t value;
    } CONFIG4;

} ADS1299_GPIOAndMiscRegType;

typedef struct {
    uint8_t Board_Id;
    ADS1299_GlobalChannelRegType  GlbChnRegs;
    ADS1299_ChannelControlRegType ChnCtrlRegs;
    ADS1299_LeadOffDetectAndCurDirCtrlRegType LeadOffDetectCDCtrlRegs;
    uint8_t LOFF_STATP;
    uint8_t LOFF_STATN;
    ADS1299_GPIOAndMiscRegType GpioMiscRegs;
} ADS1299_RegTableType;

typedef struct {
    ADS1299_HwConfigType *pHw;
    ADS1299_RegTableType RegTable[ADS1299_NUMBER_OF_USED_BOARD];
} ADS1299_GlobalConfigType;

extern void ADS1299_CmdRESET(void);
extern void ADS1299_CmdSTART(void);
extern void ADS1299_CmdSTOP(void);
extern void ADS1299_CmdWAKEUP(void);
extern void ADS1299_CmdSDATAC(void);
extern void ADS1299_CmdRDATAC(void);

extern uint8_t ADS1299_GetID(uint8_t board);
extern ADS1299_ReturnType ADS1299_ConfigChannelSettings(uint8_t board, uint8_t channel, ADS1299_SetStateType pwdn, uint8_t gain, uint8_t input_type, ADS1299_SetStateType srb2);
extern ADS1299_ReturnType ADS1299_ConfigAllChannelSettings(uint8_t board, ADS1299_SetStateType pwdn, uint8_t gain, uint8_t input_type, ADS1299_SetStateType srb2);
extern ADS1299_ReturnType ADS1299_GetAllRegsValue(uint8_t board);
extern ADS1299_ReturnType ADS1299_SetSrb1(uint8_t board, ADS1299_SetStateType srb1);
extern ADS1299_ReturnType ADS1299_GetSensorData(uint8_t board);
extern ADS1299_ReturnType ADS1299_Init(void);

extern ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg1(uint8_t board, \
                                                   ADS1299_SetStateType ndaisy_en, \
                                                   ADS1299_SetStateType clk_en, \
                                                   uint8_t data_rate);

extern ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg3(uint8_t board, \
                                                   ADS1299_SetStateType npd_refbuf, \
                                                   ADS1299_SetStateType bias_meas, \
												   ADS1299_SetStateType biasref_int, 
												   ADS1299_SetStateType npd_bias,
												   ADS1299_SetStateType bias_loff_sens, \
												   ADS1299_SetStateType bias_stat);

extern ADS1299_ReturnType ADS1299_ConfigGlobalChannelReg2(uint8_t board, \
                                                   ADS1299_SetStateType int_cal, \
												   ADS1299_SetStateType cal_amp, \
												   ADS1299_SetStateType cal_freq);

extern ADS1299_ReturnType ADS1299_ConfigBiasSensP(uint8_t board, \
	                                       uint8_t biasp1_en, \
                                           uint8_t biasp2_en, \
										   uint8_t biasp3_en, \
										   uint8_t biasp4_en, \
										   uint8_t biasp5_en, \
										   uint8_t biasp6_en, \
										   uint8_t biasp7_en, \
										   uint8_t biasp8_en);
                                           
extern ADS1299_ReturnType ADS1299_ConfigBiasSensN(uint8_t board, \
	                                       uint8_t biasn1_en, \
                                           uint8_t biasn2_en, \
										   uint8_t biasn3_en, \
										   uint8_t biasn4_en, \
										   uint8_t biasn5_en, \
										   uint8_t biasn6_en, \
										   uint8_t biasn7_en, \
										   uint8_t biasn8_en);



extern ADS1299_RegTableType ADS1299_GlbRegTableType[ADS1299_NUMBER_OF_USED_BOARD];
extern uint8_t ADS1299_EEGRawDataBuffer[ADS1299_NUMBER_OF_USED_BOARD][3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION];

// #ifdef __cplusplus
// }
// #endif


#endif
