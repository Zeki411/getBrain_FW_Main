#include <Arduino.h>
// #include <ArduinoBLE.h>
#include "main.h"
#include "ADS1299_Cfg.h"
// #include "Service_ManageBLE.h"

// #include "App_CollectStreamData.h"
#include "App_DrowsinessLevelPrediction.h"

void setup() {
  // put your setup code here, to run once:

  /* --- Init Hardware for ADS1299 --- */
  pinMode(ADS1299_O_CS,    OUTPUT);
  pinMode(ADS1299_O_RST,   OUTPUT);
  pinMode(ADS1299_O_START, OUTPUT);
  pinMode(ADS1299_O_PWDN,  OUTPUT);
  pinMode(ADS1299_I_DRDY,  INPUT);

  digitalWrite(ADS1299_O_CS, HIGH);
  digitalWrite(ADS1299_O_PWDN, HIGH);
  digitalWrite(ADS1299_O_RST, HIGH);

  // if (!BLE.begin()) {  //Begin BLE initialization
  //   Serial.println("Starting BLE failed!");
  //   digitalWrite(LEDR, LOW);
  //   delay(1000);
  //   digitalWrite(LEDR, HIGH);
        
  //   while (1); // Stop if BLE couldn't be initialized.
  // }

  /* --- Init HW for BLE --- */
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  ADS1299_Platform_Spi_Init(); // SPI Init

  /* --- Init HW for Debugger --- */
  Serial.begin(256000);

  /* --- Init BLE Service --- */
  // Service_ManageBLE_Init();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  // App_CollectStreamData_BLE_MainRun();
  // App_CollectStreamData_UART_MainRun();
  App_DrowsinessLevelPrediction_MainRun();
  // Serial.println('Hello');
  delay(1000);
}