#ifndef __GETBRAIN_BLE_SERVICE__
#define __GETBRAIN_BLE_SERVICE__

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../getBrain_common/Std_Types.h"


#define GETBRAIN_BLE_SERVICE_UUID               0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
			                                    0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x75

#define GETBRAIN_BLE_TX_CHARACTERISTIC_UUID     0xED, 0xAA, 0x20, 0x11, 0x92, 0xE7, 0x43, 0x5A, \
			                                    0xAA, 0xE9, 0x94, 0x43, 0x35, 0x6A, 0xD4, 0xD3

// #define GETBRAIN_BLE_SERVICE_UUID               0x6E,0x40,0x00,0x01,0xB5,0xA3,0xF3,0x93,0xE0,0xA9,0xE5,0x0E,0x24,0xDC,0xCA,0x9E
			                                    

// #define GETBRAIN_BLE_TX_CHARACTERISTIC_UUID     0x6E,0x40,0x00,0x03,0xB5,0xA3,0xF3,0x93,0xE0,0xA9,0xE5,0x0E,0x24,0xDC,0xCA,0x9E
			                                    


extern int getBrain_BleService_Init(void);
extern void getBrain_BleService_Send(struct bt_conn *conn, const uint8_t *data, uint16_t len);
extern Std_ReturnType getBrain_IsNotiSubscribed(struct bt_conn *conn);


#ifdef __cplusplus
}
#endif

#endif