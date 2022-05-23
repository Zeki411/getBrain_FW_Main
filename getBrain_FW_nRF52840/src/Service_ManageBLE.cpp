#include <mbed.h>
#include <ArduinoBLE.h>
#include "Service_ManageBLE.h"

#define SERVICE_MANAGEBLE_GETBRAIN_SERVICE_UUID                         "19b10001-e8f2-537e-4f6c-d104768a1211"
#define SERVICE_MANAGEBLE_COLLECTSTREAM_PACKET_CHAR_UUID                "19b10001-e8f2-537e-4f6c-d104768a1212"
#define SERVICE_MANAGEBLE_CONFIGSENSOR_PACKET_CHAR_UUID                 "19b10001-e8f2-537e-4f6c-d104768a1213"
#define SERVICE_MANAGEBLE_PREDICTION_PACKET_CHAR_UUID                   "19b10001-e8f2-537e-4f6c-d104768a1214"


BLEService Service_ManageBLE_geBrainService(SERVICE_MANAGEBLE_GETBRAIN_SERVICE_UUID);
BLECharacteristic Service_ManageBLE_CollectStreamPacketChar(SERVICE_MANAGEBLE_COLLECTSTREAM_PACKET_CHAR_UUID, BLENotify,33,(1==1));
BLECharacteristic Service_ManageBLE_ConfigSensorPacketChar(SERVICE_MANAGEBLE_CONFIGSENSOR_PACKET_CHAR_UUID, BLEWrite ,10,(1==1));
BLECharacteristic Service_ManageBLE_PredictionPacketChar(SERVICE_MANAGEBLE_PREDICTION_PACKET_CHAR_UUID, BLENotify,5,(1==1));

// typedef union
// {
//     struct{
//         uint8_t start;
//         uint8_t cmd_type;
//         uint8_t params[7];
//         uint8_t stop;
//     } Frame;
//     uint8_t Buffer[10];
// } Service_ManageBLE_ConfigSensorPacketType;

void Service_ManageBLE_Init(void)
{
    /* Set advertised local name and service UUID: */
    BLE.setLocalName("getBrain-nRF52840");
    BLE.setAdvertisedService(Service_ManageBLE_geBrainService);

    /* Add the characteristic to the service */
    Service_ManageBLE_geBrainService.addCharacteristic(Service_ManageBLE_CollectStreamPacketChar);
    Service_ManageBLE_geBrainService.addCharacteristic(Service_ManageBLE_ConfigSensorPacketChar);
    Service_ManageBLE_geBrainService.addCharacteristic(Service_ManageBLE_PredictionPacketChar);

    /* Add service */
    BLE.addService(Service_ManageBLE_geBrainService);

    /* Start Advertising */
    BLE.advertise();
    digitalWrite(LEDB, LOW);
    delay(1000);
    digitalWrite(LEDB, HIGH);

}

