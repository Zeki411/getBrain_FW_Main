#include "App_DrowsinessLevelPrediction.h"
#include "Service_ManageBLE.h"
#include "main.h"
#include "ADS1299.h"
#include "Filters.h"
#include "Filters/Butterworth.hpp"

#include "arduinoFFT.h"
#include "NeuralNetwork.h"

#define APP_DLP_NUMBER_OF_USED_CHANNEL  4
#define APP_DLP_NUMBER_OF_SAMPLE  256

#define APP_DLP_NUMBER_OF_FEATURE 5
#define APP_DLP_INPUT_DATA_NUMBER APP_DLP_NUMBER_OF_FEATURE*APP_DLP_NUMBER_OF_USED_CHANNEL
#define APP_DLP_ZERO_PADDING_COEFF 2


#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03


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

NeuralNetwork *NN;

float App_DrowsinessPrediction_InputData[APP_DLP_INPUT_DATA_NUMBER]; //relative band power

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

double vReal[APP_DLP_NUMBER_OF_SAMPLE * APP_DLP_ZERO_PADDING_COEFF]; //zero padded
double vImag[APP_DLP_NUMBER_OF_SAMPLE * APP_DLP_ZERO_PADDING_COEFF];

static void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);

void App_DrowsinessLevelPrediction_MainRun(void)
{
    /* Check Wireless Connection */
    // if (!BLE.begin()) {  //Begin BLE initialization
    //    Serial.println("Starting BLE failed!");
    //    digitalWrite(LEDR, LOW);
    //    delay(1000);
    //    digitalWrite(LEDR, HIGH);
           
    //    while (1); // Stop if BLE couldn't be initialized.
    // }

    // Service_ManageBLE_Init();

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
    // ADS1299_CmdSTART();
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

                uint16_t u16SampleCnt = 0;
                uint8_t aU8RawEegData[APP_DLP_NUMBER_OF_SAMPLE][ADS1299_BYTEDATA_RESOLUTION*ADS1299_NUMBER_OF_CHANNEL];
                
                double aDSciEegData[APP_DLP_NUMBER_OF_SAMPLE];
                double aDFeatureSet[APP_DLP_NUMBER_OF_USED_CHANNEL][APP_DLP_NUMBER_OF_FEATURE];

                double sum_of_all_bands = 0;

                memset(aU8RawEegData, 0, APP_DLP_NUMBER_OF_SAMPLE*ADS1299_BYTEDATA_RESOLUTION*ADS1299_NUMBER_OF_CHANNEL);
                memset(aDSciEegData, 0, APP_DLP_NUMBER_OF_SAMPLE);
                
                double dFs = 250; // Hz
                double dFc = 50; // Hz
                double dFn = 2 * dFc / dFs; // Normalized cut-off frequency
                auto filter = butter<6>(dFn); // Sixth-order Butterworth filter (Low-pass)

                // Start Prediction
                ADS1299_CmdSTART();
                while(u16SampleCnt != APP_DLP_NUMBER_OF_SAMPLE){
                    if(ADS1299_GetSensorData(0) == ADS1299_OK) {
                        memcpy(&aU8RawEegData[u16SampleCnt][0], &ADS1299_EEGRawDataBuffer[0][3], 24);
                        u16SampleCnt++;
                    }
                }
                ADS1299_CmdSTOP();

                // Data Converter && Filter && Multiplying Hanning Window
                for(uint8_t u8ChnCnt = 0; u8ChnCnt < APP_DLP_NUMBER_OF_USED_CHANNEL; u8ChnCnt++)
                {
                    
                    for(uint16_t u16SampleCnt = 0; u16SampleCnt < APP_DLP_NUMBER_OF_SAMPLE; u16SampleCnt++)
                    {
                        int iChannelData = (int)(((0xFF & aU8RawEegData[u16SampleCnt][3 + 3*u8ChnCnt    ]) << 16) | \
                                                 ((0xFF & aU8RawEegData[u16SampleCnt][3 + 3*u8ChnCnt  + 1]) << 8) | \
                                                 ((0xFF & aU8RawEegData[u16SampleCnt][3 + 3*u8ChnCnt  + 2])));

                        if(bitRead(iChannelData, 23) == 1){
                            iChannelData |= 0xFF000000;
                        }
                        else{
                            iChannelData &= 0x00FFFFFF;
                        }

                        aDSciEegData[u16SampleCnt] = iChannelData*ADS1299_SCALE_MV; 
                    }
                    
                    // Filter
                    // for(uint16_t u16SampleCnt = 0; u16SampleCnt < APP_DLP_NUMBER_OF_SAMPLE; u16SampleCnt++)
                    // {
                    //     aFSciEegData[u8ChnCnt][u16SampleCnt] = filter((float)aFSciEegData[u8ChnCnt][u16SampleCnt]);
                    // }

                    // Feature Extraction                    
                    // PrintVector(vReal, APP_DLP_NUMBER_OF_SAMPLE, SCL_TIME);

                    FFT.Windowing(aDSciEegData, APP_DLP_NUMBER_OF_SAMPLE, FFT_WIN_TYP_HANN, FFT_FORWARD); // Windowing

                    //zero padding
                    memset(vImag, 0, APP_DLP_NUMBER_OF_SAMPLE*APP_DLP_ZERO_PADDING_COEFF);
                    memset(vReal, 0, APP_DLP_NUMBER_OF_SAMPLE*APP_DLP_ZERO_PADDING_COEFF);
                    for (uint16_t u16cnt = 0 ; u16cnt < APP_DLP_NUMBER_OF_SAMPLE; u16cnt++) // can be optimized later
                    {
                        vReal[u16cnt] = aDSciEegData[u16cnt];
                    }
                    

                    FFT.Compute(vReal, vImag, APP_DLP_NUMBER_OF_SAMPLE, FFT_FORWARD); /* Compute FFT */
                    // Serial.println("Computed Real values:");
                    // PrintVector(vReal, APP_DLP_NUMBER_OF_SAMPLE, SCL_INDEX);

                    FFT.ComplexToMagnitude(vReal, vImag, APP_DLP_NUMBER_OF_SAMPLE);
                    Serial.println("Computed magnitudes:");
                    PrintVector(vReal, (APP_DLP_NUMBER_OF_SAMPLE >> 1), SCL_FREQUENCY);

                    // Calculating PSD
                    sum_of_all_bands = 0;
                    for(int freqbincnt = 4*APP_DLP_ZERO_PADDING_COEFF; freqbincnt <= 30*APP_DLP_ZERO_PADDING_COEFF; freqbincnt++) // because zero padding
                    {
                        sum_of_all_bands += vReal[freqbincnt];
                        if (freqbincnt < 6*APP_DLP_ZERO_PADDING_COEFF) 
                        App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 0] += vReal[freqbincnt]; //ltheta

                        if ((6*APP_DLP_ZERO_PADDING_COEFF <= freqbincnt) && (freqbincnt< 8*APP_DLP_ZERO_PADDING_COEFF))
                        App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 1] += vReal[freqbincnt]; //htheta

                        if ((8*APP_DLP_ZERO_PADDING_COEFF <= freqbincnt) && (freqbincnt< 10*APP_DLP_ZERO_PADDING_COEFF))
                        App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 2] += vReal[freqbincnt]; //lalpha

                        if ((10*APP_DLP_ZERO_PADDING_COEFF <= freqbincnt) && (freqbincnt< 13*APP_DLP_ZERO_PADDING_COEFF))
                        App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 3] += vReal[freqbincnt]; //halpha

                        if ((13*APP_DLP_ZERO_PADDING_COEFF <= freqbincnt) && (freqbincnt <= 30*APP_DLP_ZERO_PADDING_COEFF))
                        App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 4] += vReal[freqbincnt]; //beta
                    }

                    App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 0] = \
                                        (double)App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 0] / sum_of_all_bands;
                    App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 1] = \
                                        (double)App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 1] / sum_of_all_bands;
                    App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 2] = \
                                        (double)App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 2] / sum_of_all_bands;
                    App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 3] = \
                                        (double)App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 3] / sum_of_all_bands;
                    App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 4] = \
                                        (double)App_DrowsinessPrediction_InputData[u8ChnCnt*APP_DLP_NUMBER_OF_USED_CHANNEL + 4] / sum_of_all_bands;


                    // Doing Inference 
                    memcpy(NN->getInputBuffer(), &App_DrowsinessPrediction_InputData[0], APP_DLP_NUMBER_OF_FEATURE * APP_DLP_NUMBER_OF_USED_CHANNEL);
                    float result[2];
                    NN->predict(result);
                }

                

                // Prediction

                // Return Result

                delay(5000);

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

static void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    double samplingFrequency = 250;
    uint16_t samples = 256;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}