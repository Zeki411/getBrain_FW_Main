#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <nrfx_gpiote.h>
#include <helpers/nrfx_gppi.h>
#if defined(DPPI_PRESENT)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif

// #include <sys/ring_buffer.h>

#include "../getBrain_BleService/getBrain_BleService.h"
#include "../driver_libs/ads1299/ADS1299.h"
#include "../driver_libs/ads1299/ADS1299_Cfg.h"
#include "../driver_libs/circular_buffer/CircularBuffer.h"

#define LED_GREEN	            DT_GPIO_PIN(DT_ALIAS(led3), gpios)

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

static void App_CollectStream_BleConnected(struct bt_conn *conn, uint8_t err);
static void App_CollectStream_BleDisconnected(struct bt_conn *conn, uint8_t reason);
static bool App_CollectStream_BleParamReq(struct bt_conn *conn, struct bt_le_conn_param *param);
static void App_CollectStream_BleParamUpdated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);
static void App_CollectStream_BleReady(int err);
static void App_CollectStream_BleError(void);

typedef union
{
    struct
    {
//        uint8_t start;
        uint16_t nb_sample;
        uint8_t eeg_data[24];
        // uint8_t aux_data[6];
//        uint8_t stop;
    } frame;
    uint8_t buffer[26];
} App_CollectStream_SensorDataPacketType;

App_CollectStream_SensorDataPacketType App_CollectStream_SensorPacket;

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static const struct bt_data App_CollectStream_Ad[] = 
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data App_CollectStream_Sd[] = 
{
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, GETBRAIN_BLE_SERVICE_UUID),
};

static struct bt_conn_cb App_CollectStream_ConnCallbacks = 
{
	.connected				= App_CollectStream_BleConnected,
	.disconnected   		= App_CollectStream_BleDisconnected,
	.le_param_req			= App_CollectStream_BleParamReq,
	.le_param_updated		= App_CollectStream_BleParamUpdated,
};

struct bt_conn *App_CollectStream_BleConnection;

static void App_CollectStream_BleConnected(struct bt_conn *conn, uint8_t err)
{
	struct bt_conn_info info; 
	uint8_t aU8Addr[BT_ADDR_LE_STR_LEN];

	App_CollectStream_BleConnection = conn;

	if (err) 
	{
		printk("Connection failed (err %u)\n", err);
		return;
	}
	else if(bt_conn_get_info(conn, &info))
	{
		printk("Could not parse connection info\n");
	}
	else
	{
		bt_addr_le_to_str(bt_conn_get_dst(conn), (char *)aU8Addr, sizeof(aU8Addr));
		
		printk("Connection parameters updated!	\n\
		Connected to: %s						\n\
		New Connection Interval: %u				\n\
		New Slave Latency: %u					\n\
		New Connection Supervisory Timeout: %u	\n\
		New Connection Transmit PHY: %u	\n"
		, (char *)aU8Addr, info.le.interval, info.le.latency, info.le.timeout, info.le.phy->tx_phy);
	}
}

static void App_CollectStream_BleDisconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);
}

static bool App_CollectStream_BleParamReq(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	//If acceptable params, return true, otherwise return false.

	return true; 
}

static void App_CollectStream_BleParamUpdated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
	struct bt_conn_info info; 
	uint8_t aU8Addr[BT_ADDR_LE_STR_LEN];
	
	if(bt_conn_get_info(conn, &info))
	{
		printk("Could not parse connection info\n");
	}
	else
	{
		bt_addr_le_to_str(bt_conn_get_dst(conn), (char *)aU8Addr, sizeof(aU8Addr));

		printk("Connection parameters updated!	\n\
		Connected to: %s						\n\
		New Connection Interval: %u				\n\
		New Slave Latency: %u					\n\
		New Connection Supervisory Timeout: %u	\n\
		New Connection Transmit PHY: %u	\n"
		, (char *)aU8Addr, info.le.interval, info.le.latency, info.le.timeout, info.le.phy->tx_phy);
	}
}

static void App_CollectStream_BleReady(int err)
{
	if (err) 
	{
		printk("BLE init failed with error code %d\n", err);
		return;
	}

	//Configure connection callbacks
	bt_conn_cb_register(&App_CollectStream_ConnCallbacks);

	//Initalize services
	err = getBrain_BleService_Init();

	if (err) 
	{
		printk("Failed to init LBS (err:%d)\n", err);
		return;
	}

	//Start advertising
	err = bt_le_adv_start(BT_LE_ADV_CONN, \
                          App_CollectStream_Ad, ARRAY_SIZE(App_CollectStream_Ad), \
			              App_CollectStream_Sd, ARRAY_SIZE(App_CollectStream_Sd));

	if (err) 
	{
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");

	k_sem_give(&ble_init_ok);
}

static void App_CollectStream_BleError(void)
{
	while (true) {
		printk("Error!\n");
		/* Spin for ever */
		k_sleep(K_MSEC(1000)); //1000ms
	}
}

void App_CollectStream_BLE_MainRun(void)
{
    int err = 0;
    Std_ReturnType ret = E_OK;
    uint8_t aU8BleTxMsg[50]; 
    memset(aU8BleTxMsg, 0, 50);

	/* Hw Init */
	err = nrfx_gpiote_init(0);
	if (err != NRFX_SUCCESS) {
		// LOG_ERR("nrfx_gpiote_init error: %08x", err);
		return;
	}

	nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(0);

	err = nrfx_gpiote_out_init(LED_GREEN, &out_config);
	if (err != NRFX_SUCCESS) {
		// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
		return;
	}
	nrfx_gpiote_out_set(LED_GREEN);

	ADS1299_Platform_Hw_ConfigGpio();
    ADS1299_Platform_Hw_ConfigSpi(0); // Polling to Config

    err = bt_enable(App_CollectStream_BleReady);
    if (err) 
	{
		printk("BLE initialization failed\n");
		App_CollectStream_BleError(); //Catch error
	}

	/* 	Bluetooth stack should be ready in less than 100 msec. 								\
																							\
		We use this semaphore to wait for bt_enable to call bt_ready before we proceed 		\
		to the main loop. By using the semaphore to block execution we allow the RTOS to 	\
		execute other tasks while we wait. */	
	err = k_sem_take(&ble_init_ok, K_MSEC(500));

	if (!err) 
	{
		printk("Bluetooth initialized\n");
	} else 
	{
		printk("BLE initialization did not complete in time\n");
		App_CollectStream_BleError(); //Catch error
	}

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

    while(getBrain_IsNotiSubscribed(App_CollectStream_BleConnection) != E_OK)
    {
        k_msleep(10);
    }
	
    ADS1299_CmdSTART();
    ADS1299_CmdRDATAC();

	/* Start Aquisition */
	
	ADS1299_Platform_Hw_ConfigSpi(2); // DMA
    //ADS1299_Platform_Hw_ConfigDrdy(0);
	ADS1299_Platform_Hw_ConfigDrdy_UsingPPI();

    while(1)
    {
		// if(ADS1299_GetSensorData(0) == ADS1299_OK)
        // {
		// 	//memcpy(&App_CollectStream_SensorPacket.frame.eeg_data[0], &ADS1299_EEGRawDataBuffer[0][3], 24);
		// 	ret = CircularBuffer_Push(&ADS1299_Platform_EegCirBuf, &ADS1299_EEGRawDataBuffer[0][3], \
		// 	                            24);

		// 	ret = CircularBuffer_Pop(&ADS1299_Platform_EegCirBuf, &App_CollectStream_SensorPacket.frame.eeg_data[0], \
		// 	                            24);

		// 	getBrain_BleService_Send(App_CollectStream_BleConnection, (uint8_t *)&App_CollectStream_SensorPacket.buffer[0], 33);
		// 	App_CollectStream_SensorPacket.frame.nb_sample++;
		// }		

		
        if(ADS1299_Platform_EegCirBuf.buffer_status == CIRCULAR_BUFFER_STATE_FULL_BUFFER)
		{
			//printk("[App_CollectStream_EegCirBuf] Error: EEG Circular Buffer is full \r\n");
			nrfx_gpiote_out_clear(LED_GREEN); // Led green on
		}
		else
		{
			nrfx_gpiote_out_set(LED_GREEN); // Led green off
		}
        
		ret = CircularBuffer_Pop(&ADS1299_Platform_EegCirBuf, &App_CollectStream_SensorPacket.frame.eeg_data[0], 24);
		if(ret == E_OK)
		{
			getBrain_BleService_Send(App_CollectStream_BleConnection, (uint8_t *)&App_CollectStream_SensorPacket.buffer[0], 33);
			App_CollectStream_SensorPacket.frame.nb_sample++;

			if(App_CollectStream_SensorPacket.frame.nb_sample%2000 == 0)
			{
				printk("Circuit_Buffer_Length: %d\r\n", ADS1299_Platform_EegCirBuf.buffer_length);
			}
		}
    }
}



