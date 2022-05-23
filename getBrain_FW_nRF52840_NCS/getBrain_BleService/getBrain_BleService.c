#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/addr.h>
#include <bluetooth/gatt.h>

#include "../getBrain_Common/Std_Types.h"
#include "getBrain_BleService.h"

#define GETBRAIN_BT_UUID_SERVICE      BT_UUID_DECLARE_128(GETBRAIN_BLE_SERVICE_UUID)
#define GETBRAIN_BT_UUID_SERVICE_TX   BT_UUID_DECLARE_128(GETBRAIN_BLE_TX_CHARACTERISTIC_UUID)

#define MAX_TRANSMIT_SIZE 50

uint8_t getBrain_TxBuffer[MAX_TRANSMIT_SIZE];

static void getBrain_OnSent(struct bt_conn *conn, void *user_data);
void getBrain_BleService_OnCccdChanged(const struct bt_gatt_attr *attr, uint16_t value);

/* getBrain Service Declaration and Registration */
BT_GATT_SERVICE_DEFINE(getBrain_BleService,
BT_GATT_PRIMARY_SERVICE(GETBRAIN_BT_UUID_SERVICE),
BT_GATT_CHARACTERISTIC(GETBRAIN_BT_UUID_SERVICE_TX,
			       BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
                   NULL, NULL, NULL),
BT_GATT_CCC(getBrain_BleService_OnCccdChanged,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* This function is called whenever a Notification has been sent by the TX Characteristic */
static void getBrain_OnSent(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);

    const bt_addr_le_t * addr = bt_conn_get_dst(conn);

	// printk("Data sent to Address 0x %02X %02X %02X %02X %02X %02X \n", addr->a.val[0]
    //                                                                 , addr->a.val[1]
    //                                                                 , addr->a.val[2]
    //                                                                 , addr->a.val[3]
    //                                                                 , addr->a.val[4]
    //                                                                 , addr->a.val[5]);

}

/* This function is called whenever the CCCD register has been changed by the client*/
void getBrain_BleService_OnCccdChanged(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    switch(value)
    {
        case BT_GATT_CCC_NOTIFY: 
            // Start sending stuff!
            break;

        case BT_GATT_CCC_INDICATE: 
            // Start sending stuff via indications
            break;

        case 0: 
            // Stop sending stuff
            break;
        
        default: 
            printk("Error, CCCD has been set to an invalid value");     
    }
}

int getBrain_BleService_Init(void)
{
     int err = 0;

    memset(&getBrain_TxBuffer, 0, MAX_TRANSMIT_SIZE);

    return err;
}

/* This function sends a notification to a Client with the provided data,
given that the Client Characteristic Control Descripter has been set to Notify (0x1).
It also calls the on_sent() callback if successful*/
void getBrain_BleService_Send(struct bt_conn *conn, const uint8_t *data, uint16_t len)
{
    /* 
    The attribute for the TX characteristic is used with bt_gatt_is_subscribed 
    to check whether notification has been enabled by the peer or not.
    Attribute table: 0 = Service, 1 = Primary service, 2 = TX, 4 = CCC.
    */
    const struct bt_gatt_attr *attr = &getBrain_BleService.attrs[2]; 

    struct bt_gatt_notify_params params = 
    {
        .uuid   = GETBRAIN_BT_UUID_SERVICE_TX,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = getBrain_OnSent
    };

    // Check whether notifications are enabled or not
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) 
    {
        // Send the notification
	    if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled on the selected attribute\n");
    }
}

Std_ReturnType getBrain_IsNotiSubscribed(struct bt_conn *conn)
{
    const struct bt_gatt_attr *attr = &getBrain_BleService.attrs[2]; 

    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) 
    {
        return E_OK;
    }
    else
    {
        return E_NOT_OK;
    }
}