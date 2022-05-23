#include "App_DrowsinessLevelPrediction.h"
#include "Service_ManageBLE.h"
#include "main.h"
#include "ADS1299.h"
#include "Filters.h"
#include "Filters/Butterworth.hpp"

#include "arduinoFFT.h"
#include "NeuralNetwork.h"

#define APP_DLP_NUMBER_OF_SAMPLE  250

#define APP_DLP_INPUT_DATA_NUMBER 250*3 

typedef union
{
    struct
    {
        uint8_t start;
        uint8_t prediction_result;
        uint16_t accuracy_percent;
        uint8_t stop;
    } frame;
    uint8_t buffer[5];
} App_DrowsinessLevelPrediction_PacketType;

App_DrowsinessLevelPrediction_PacketType App_DrowsinessLevelPrediction_Packet;

float App_DrowsinessPrediction_InputData[APP_DLP_INPUT_DATA_NUMBER]; 


void App_DrowsinessLevelPrediction_MainRun(void)
{
    /* Check Wireless Connection */
    if (!BLE.begin()) {  //Begin BLE initialization
       Serial.println("Starting BLE failed!");
       digitalWrite(LEDR, LOW);
       delay(1000);
       digitalWrite(LEDR, HIGH);
           
       while (1); // Stop if BLE couldn't be initialized.
    }

    Service_ManageBLE_Init();

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
    ADS1299_CmdSTOP();
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

            ADS1299_CmdSTART();

            while (central.connected()) {

                uint8_t u8SampleCnt = 0;
                uint8_t aU8RawEegData[APP_DLP_NUMBER_OF_SAMPLE][ADS1299_BYTEDATA_RESOLUTION*ADS1299_NUMBER_OF_CHANNEL];
                float aFSciEegData[ADS1299_NUMBER_OF_CHANNEL][APP_DLP_NUMBER_OF_SAMPLE];

                memset(aU8RawEegData, 0, APP_DLP_NUMBER_OF_SAMPLE*ADS1299_BYTEDATA_RESOLUTION*ADS1299_NUMBER_OF_CHANNEL);
                memset(aFSciEegData, 0, APP_DLP_NUMBER_OF_SAMPLE*ADS1299_NUMBER_OF_CHANNEL);


                double dFs = 250; // Hz
                double dFc = 50; // Hz
                double dFn = 2 * dFc / dFs; // Normalized cut-off frequency
                auto filter = butter<6>(dFn); // Sixth-order Butterworth filter (Low-pass)

                // Start Prediction
                ADS1299_CmdSTART();
                while(u8SampleCnt != 250){
                    if(ADS1299_GetSensorData(0) == ADS1299_OK) {
                        memcpy(&aU8RawEegData[u8SampleCnt][0], &ADS1299_EEGRawDataBuffer[0][3], 24);
                        u8SampleCnt++;
                    }
                }
                ADS1299_CmdSTOP();

                // Data Converter && Filter && Multiplying Hanning Window
                for(uint8_t u8ChnCnt = 0; u8ChnCnt < ADS1299_NUMBER_OF_CHANNEL; u8ChnCnt++)
                {
                    for(uint8_t u8SampleCnt = 0; u8SampleCnt < APP_DLP_NUMBER_OF_SAMPLE; u8SampleCnt++)
                    {
                        int iChannelData = (int)(((0xFF & aU8RawEegData[u8SampleCnt][3 + 3*u8SampleCnt    ]) << 16) | \
                                                 ((0xFF & aU8RawEegData[u8SampleCnt][3 + 3*u8SampleCnt  + 1]) << 8) | \
                                                 ((0xFF & aU8RawEegData[u8SampleCnt][3 + 3*u8SampleCnt  + 2])));

                        if(bitRead(iChannelData, 23) == 1){
                            iChannelData |= 0xFF000000;
                        }
                        else{
                            iChannelData &= 0x00FFFFFF;
                        }

                        aFSciEegData[u8ChnCnt][u8SampleCnt] = iChannelData*ADS1299_SCALE_MV; 
                    }
                    
                    // Baseline Correction 
                    // double dSignalSum = 0;
                    // for(uint16_t u8SampleCnt = 0; u8SampleCnt < 1000; u8SampleCnt++)
                    // {
                    //    dSignalSum += aFSciEegData[0][u8SampleCnt];
                    // }
                    // for(uint16_t u8SampleCnt = 0; u8SampleCnt < 1000; u8SampleCnt++)
                    // {
                    //    aFSciEegData[0][u8SampleCnt] -= (float)(dSignalSum/1000);
                    // }
                    
                    //Filter
                    for(uint16_t u8SampleCnt = 0; u8SampleCnt < 1000; u8SampleCnt++)
                    {
                        aFSciEegData[0][u8SampleCnt] = filter(aFSciEegData[0][u8SampleCnt]);
                    }

                    // Multiplying Hanning Window
                    double dHanningPointMultiplier = 0;
                    for(uint16_t u8SampleCnt = 0; u8SampleCnt < 1000; u8SampleCnt++)
                    {
                        dHanningPointMultiplier = 0.5 * (1 - cos(2*PI*u8SampleCnt/(1000-1)));
                        aFSciEegData[0][u8SampleCnt] = aFSciEegData[0][u8SampleCnt] * dHanningPointMultiplier;
                    }



                }

                // Feature Extraction
                

                // Prediction

                // Return Result


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