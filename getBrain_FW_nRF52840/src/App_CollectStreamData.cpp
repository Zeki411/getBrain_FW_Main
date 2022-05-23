#include "App_CollectStreamData.h"
#include "Service_ManageBLE.h"
#include "ADS1299.h"
#include "main.h"

#define CONFIG_PROFILE_1

typedef union
{
    struct
    {
        uint8_t start;
        uint8_t nb_sample;
        uint8_t eeg_data[24];
        uint8_t aux_data[6];
        uint8_t stop;
    } frame;
    uint8_t buffer[33];
} App_CollectStream_SensorDataPacketType;

App_CollectStream_SensorDataPacketType App_CollectStream_SensorPacket;


void App_CollectStreamData_BLE_MainRun(void)
{
    /* Check Connection */


    /* Initialiaze Parameter the ADS1299 */
    ADS1299_Init();
    ADS1299_ConfigGlobalChannelReg1(0, \
                                ADS1299_RESET, \
                                ADS1299_RESET, \
                                ADS1299_DR_250_SPS \
                                );

#if(USING_BIAS_DRIVE_CIRCUIT == 1)
    ADS1299_ConfigGlobalChannelReg2(0, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET \
                                    );                              

    ADS1299_ConfigGlobalChannelReg3(0, \
                                    ADS1299_SET, \ 
                                    ADS1299_RESET, \
                                    ADS1299_SET,   \
                                    ADS1299_SET, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET  \
                                    );
    
   ADS1299_ConfigBiasSensP(0, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET \
                            );
    
    ADS1299_ConfigBiasSensN(0, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET \
                            );
#endif

    ADS1299_SetSrb1(0, ADS1299_SET);
    ADS1299_ConfigAllChannelSettings(0, ADS1299_RESET, ADS1299_GAIN24, ADS1299_INPUT_NORMAL, ADS1299_RESET);
    ADS1299_CmdSTART();
    ADS1299_CmdRDATAC();

    while(1)
    {

        BLEDevice central = BLE.central();

        // If a central is connected to peripheral:
        if (central) {
            Serial.print("Connected to central: ");

            // Print the central's MAC address:
            Serial.println(central.address());

            digitalWrite(LEDB, HIGH);
            delay(100);
            digitalWrite(LEDB, LOW);
            delay(100);
            digitalWrite(LEDB, HIGH);

            while (central.connected()) {
                if(ADS1299_GetSensorData(0) == ADS1299_OK)
                { 
                    //Send Data through BLE
                    memcpy(&App_CollectStream_SensorPacket.frame.eeg_data[0], &ADS1299_EEGRawDataBuffer[0][3], 24);
                    Service_ManageBLE_CollectStreamPacketChar.writeValue(App_CollectStream_SensorPacket.buffer,33);
                    App_CollectStream_SensorPacket.frame.nb_sample++;
                }
            }

            // When the central disconnects, print it out:
            Serial.print("Disconnected from central: ");
            Serial.println(central.address());    
            digitalWrite(LEDB, HIGH);
            delay(100);
            digitalWrite(LEDB, LOW);
            delay(100);
            digitalWrite(LEDB, HIGH);
        }

    }
}

void App_CollectStreamData_UART_MainRun(void)
{
    char aCMsg[100]; 
    int iChannelDataBuf[8];
    
    memset(aCMsg, 0, 100);

    /* Setup Connection */
    // mbed::BufferedSerial App_HwSerial(P1_3,P1_10,115200);
    // App_HwSerial.set_blocking(false);
    // App_HwSerial.enable_input(false);

    // Serial1.begin(115200);
    // _SerialUSB

    /* Initialiaze Parameter the ADS1299 */
    ADS1299_Init();
    ADS1299_ConfigGlobalChannelReg1(0, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET, \
                                    ADS1299_DR_250_SPS \
                                    );

#if(USING_BIAS_DRIVE_CIRCUIT == 1)
    ADS1299_ConfigGlobalChannelReg2(0, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET \
                                    );                              

    ADS1299_ConfigGlobalChannelReg3(0, \
                                    ADS1299_SET, \ 
                                    ADS1299_RESET, \
                                    ADS1299_SET,   \
                                    ADS1299_SET, \
                                    ADS1299_RESET, \
                                    ADS1299_RESET  \
                                    );
    
   ADS1299_ConfigBiasSensP(0, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET \
                            );
    
    ADS1299_ConfigBiasSensN(0, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET, \
                            ADS1299_SET \
                            );
#endif
    
    ADS1299_ConfigAllChannelSettings(0, ADS1299_RESET, ADS1299_GAIN24, ADS1299_INPUT_NORMAL, ADS1299_RESET);

    ADS1299_SetSrb1(0, ADS1299_SET);
   
   	/* Update All Register Data */
	ADS1299_GetAllRegsValue(0);

    ADS1299_CmdSTART();
    ADS1299_CmdRDATAC();

    while(1)
    {
        if(ADS1299_GetSensorData(0) == ADS1299_OK)
        {
            memset(iChannelDataBuf, 0, 8);

            //Send Data through UART blocking
            for(uint8_t u8Count = 0; u8Count < 8; u8Count++)
            {
                iChannelDataBuf[u8Count] = (int)(((0xFF & ADS1299_EEGRawDataBuffer[0][3 + u8Count*3 + 0]) << 16) | \
                                                 ((0xFF & ADS1299_EEGRawDataBuffer[0][3 + u8Count*3 + 1]) <<  8) | \
                                                 ((0xFF & ADS1299_EEGRawDataBuffer[0][3 + u8Count*3 + 2])));
                
                if(bitRead(iChannelDataBuf[u8Count], 23) == 1)
                {
                    iChannelDataBuf[u8Count] |= 0xFF000000;
                }
                else
                {
                    iChannelDataBuf[u8Count] &= 0x00FFFFFF;
                }
            }

            // sprintf(aCMsg, "%d,%d,%d,%d,%d,%d,%d,%d\r\n", iChannelDataBuf[0], \
            //                                               iChannelDataBuf[1], \
            //                                               iChannelDataBuf[2], \
            //                                               iChannelDataBuf[3], \
            //                                               iChannelDataBuf[4], \
            //                                               iChannelDataBuf[5], \
            //                                               iChannelDataBuf[6], \
            //                                               iChannelDataBuf[7]);


            sprintf(aCMsg, "%f,%f,%f,%f,%f,%f,%f,%f\r\n", (float)(iChannelDataBuf[0]*0.02235), \
                                                          (float)(iChannelDataBuf[1]*0.02235), \
                                                          (float)(iChannelDataBuf[2]*0.02235), \
                                                          (float)(iChannelDataBuf[3]*0.02235), \
                                                          (float)(iChannelDataBuf[4]*0.02235), \
                                                          (float)(iChannelDataBuf[5]*0.02235), \
                                                          (float)(iChannelDataBuf[6]*0.02235), \
                                                          (float)(iChannelDataBuf[7]*0.02235));

            // App_HwSerial.write(aCMsg, sizeof(aCMsg));
            Serial.print(aCMsg);
        }
    }
}