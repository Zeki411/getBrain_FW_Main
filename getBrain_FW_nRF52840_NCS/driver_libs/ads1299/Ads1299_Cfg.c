#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <soc.h>

#include <device.h>
#include <devicetree.h>

#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>

#include <nrfx_gpiote.h>
#include <nrfx_spi.h>
#include <nrfx_spim.h>

#include <helpers/nrfx_gppi.h>
#if defined(DPPI_PRESENT)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif

#include "Ads1299.h"
#include "Ads1299_Cfg.h"

#include "../../getBrain_Common/Std_Types.h"
#include "../circular_buffer/CircularBuffer.h"

// #define ADS1299I_DRDY	DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299idrdy), gpios, 0)
// #define ADS1299O_RST	(32*0+27)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299orst), gpios, 0)
// #define ADS1299O_START	(32*1+12)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ostart), gpios, 1)
// #define ADS1299O_CS	    (32*1+2)    //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ocs), gpios, 1)
// #define ADS1299O_PWDN   (32*1+15)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299opwdn), gpios, 1)

// #define ADS1299I_DRDY	DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299idrdy), gpios, 0)
// #define ADS1299O_RST	(32*0+27)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299orst), gpios, 0)
// #define ADS1299O_START	(32*1+12)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ostart), gpios, 1)
// #define ADS1299O_CS	    (32*1+2)    //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ocs), gpios, 1)
// #define ADS1299O_PWDN   (32*1+15)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299opwdn), gpios, 1)
// #define ADS1299_SPI_SCK   13  //14 
// #define ADS1299_SPI_MOSI  33  //13 
// #define ADS1299_SPI_MISO  40  //15 


#define ADS1299I_DRDY	DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299idrdy), gpios, 0)
#define ADS1299O_RST	(32*0+5)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299orst), gpios, 0)
#define ADS1299O_START	(32*0+4)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ostart), gpios, 1)
#define ADS1299O_CS	    (32*0+8)    //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299ocs), gpios, 1)
#define ADS1299O_PWDN   (32*1+19)   //DT_GPIO_PIN_BY_IDX(DT_ALIAS(ads1299opwdn), gpios, 1)
#define ADS1299_SPI_SCK   14 
#define ADS1299_SPI_MOSI  13 
#define ADS1299_SPI_MISO  15 

static void ADS1299_Platform_Spi_TxRx(uint8_t *pTxData, uint16_t tx_len, uint8_t *pRxData, uint16_t rx_len, uint8_t async);
static void ADS1299_Platform_IoCs_Write(uint8_t board, uint8_t level);
static void ADS1299_Platform_IoStart_Write(uint8_t level);
static void ADS1299_Platform_IoReset_Write(uint8_t level);
static void ADS1299_Platform_IoPwdn_Write(uint8_t level);
static uint8_t ADS1299_Platform_IoDrdy_Read();
static void ADS1299_Platform_Delay(uint16_t ms);
static void ADS1299_Platform_Hw_DrdyEventCb(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
static void ADS1299_Platform_SpiInit(nrfx_spi_evt_handler_t spi_cb);
static void ADS1299_Platform_Hw_SpiDeInit(void);
static void ADS1299_Platform_SpimInit(nrfx_spim_evt_handler_t spim_cb);
static void ADS1299_Platform_Hw_SpimDeInit(void);
static void ADS1299_Platform_Spi_TxRxDMA(uint8_t *pTxData, uint16_t tx_len, uint8_t *pRxData, uint16_t rx_len);
static void ADS1299_Platform_SpiEventCb(nrfx_spi_evt_t const * p_event,void * p_context);

/* === Define HW Interface for ADS1299 === */
ADS1299_HwConfigType ADS1299_Hw = {
    .Spi_TransmitReceive    = &ADS1299_Platform_Spi_TxRx,
	.Spi_TransmitReceiveDMA = &ADS1299_Platform_Spi_TxRxDMA,
    .IoCs_Write             = &ADS1299_Platform_IoCs_Write,
    .IoStart_Write          = &ADS1299_Platform_IoStart_Write,
    .IoReset_Write          = &ADS1299_Platform_IoReset_Write,
    .IoPwdn_Write           = &ADS1299_Platform_IoPwdn_Write,
    .IoDrdy_Read            = &ADS1299_Platform_IoDrdy_Read,
    .Delay_Ms               = &ADS1299_Platform_Delay,
};
//NRFX_SPI_PIN_NOT_USED, //34,
static const nrfx_spi_config_t spi_config = {
	.sck_pin = ADS1299_SPI_SCK,
	.mosi_pin = ADS1299_SPI_MOSI,
	.miso_pin = ADS1299_SPI_MISO,
	.ss_pin = ADS1299O_CS, 
	.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
	.orc          = 0xFF,
	.frequency    = NRF_SPI_FREQ_4M,
	.mode         = NRF_SPI_MODE_1,
	.bit_order    = NRF_SPI_BIT_ORDER_MSB_FIRST,
	.miso_pull    = NRF_GPIO_PIN_NOPULL,            
};
//NRFX_SPIM_PIN_NOT_USED, //34,
static const nrfx_spim_config_t spim_config = {
	.sck_pin = ADS1299_SPI_SCK,
	.mosi_pin = ADS1299_SPI_MOSI,
	.miso_pin = ADS1299_SPI_MISO,
	.ss_pin = NRFX_SPIM_PIN_NOT_USED, //ADS1299O_CS, 
	.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
	.orc          = 0xFF,
	.frequency    = NRF_SPIM_FREQ_4M,
	.mode         = NRF_SPIM_MODE_1,
	.bit_order    = NRF_SPIM_BIT_ORDER_MSB_FIRST,
	.miso_pull    = NRF_GPIO_PIN_NOPULL,   
	.ss_active_high = false
};

static const nrfx_spi_t spi_dev = NRFX_SPI_INSTANCE(1);

static const nrfx_spim_t spim_dev = NRFX_SPIM_INSTANCE(1);

volatile CircularBuffer_t ADS1299_Platform_EegCirBuf =
{
    .buffer_status = CIRCULAR_BUFFER_STATE_IDLE,
    .write_index = 0,
    .read_index = 0,
    .buffer_length = 0,
    .max_slots = CIRCULAR_BUFFER_MAX_SLOTS
};

static void ADS1299_Platform_Spi_TxRx(uint8_t *pTxData, uint16_t tx_len, uint8_t *pRxData, uint16_t rx_len, uint8_t async)
{
	nrfx_err_t err;

	nrfx_spi_xfer_desc_t spi_buffer = {
		.p_tx_buffer = pTxData,\
		.tx_length = tx_len,\
		.p_rx_buffer = pRxData,\
		.rx_length = rx_len
	};

	err =  nrfx_spi_xfer(&spi_dev, &spi_buffer, 0); 
}

static void ADS1299_Platform_Spi_TxRxDMA(uint8_t *pTxData, uint16_t tx_len, uint8_t *pRxData, uint16_t rx_len)
{
	nrfx_err_t err;

	nrfx_spim_xfer_desc_t spim_buffer = {
		.p_tx_buffer = pTxData,\
		.tx_length = tx_len,\
		.p_rx_buffer = pRxData,\
		.rx_length = rx_len
	};

	err =  nrfx_spim_xfer(&spim_dev, &spim_buffer, 0); 
}

static void ADS1299_Platform_IoCs_Write(uint8_t board, uint8_t level)
{
    if(board == 0)
    {
		// Empty
    }
}

static void ADS1299_Platform_IoStart_Write(uint8_t level)
{
	if(level == ADS1299_IO_LOW)
	{
		nrf_gpio_pin_write(ADS1299O_START, 0);
	}
	else
	{
		nrf_gpio_pin_write(ADS1299O_START, 1);
	}
}

static void ADS1299_Platform_IoReset_Write(uint8_t level)
{
	if(level == ADS1299_IO_LOW)
	{
		nrf_gpio_pin_write(ADS1299O_RST, 0);
	}
	else
	{
		nrf_gpio_pin_write(ADS1299O_RST, 1);
	}
}

static void ADS1299_Platform_IoPwdn_Write(uint8_t level)
{
    if(level == ADS1299_IO_LOW)
	{
		nrf_gpio_pin_write(ADS1299O_PWDN, 0);
	}
	else
	{
		nrf_gpio_pin_write(ADS1299O_PWDN, 1);
	}
}

static uint8_t ADS1299_Platform_IoDrdy_Read()
{
	if(nrfx_gpiote_in_is_set(ADS1299I_DRDY) == true)
	{
		return ADS1299_IO_HIGH;
	}
	else
	{
		return ADS1299_IO_LOW;
	}
	
}

static void ADS1299_Platform_Delay(uint16_t ms) 
{
    k_msleep(ms);
}

static void ADS1299_Platform_SpiEventCb(nrfx_spi_evt_t const * p_event,void * p_context)
{
	Std_ReturnType ret = E_OK;
	ARG_UNUSED(p_context);

	switch (p_event->type) {
		case NRFX_SPI_EVENT_DONE:
		{
		    // Inform application that transfer is completed.
		    ret = CircularBuffer_Push(&ADS1299_Platform_EegCirBuf, &ADS1299_EEGRawDataBuffer[0][3], 24);
            break;
		}
		default:
            // No implementation needed.
		    break;
    }
}

static void ADS1299_Platform_SpimEventCb(nrfx_spim_evt_t const * p_event,void * p_context)
{
	Std_ReturnType ret = E_OK;
	ARG_UNUSED(p_context);

	switch (p_event->type) {
		case NRFX_SPIM_EVENT_DONE:
		{
		    // Inform application that transfer is completed.
		    ret = CircularBuffer_Push(&ADS1299_Platform_EegCirBuf, &ADS1299_EEGRawDataBuffer[0][3], 24);
            break;
		}
		default:
            // No implementation needed.
		    break;
    }
}

static void ADS1299_Platform_SpiInit(nrfx_spi_evt_handler_t spi_cb)
{
	nrfx_err_t nrfx_err;

	nrfx_err =  nrfx_spi_init(
		&spi_dev,
		&spi_config,
		spi_cb,
		NULL);
	
	if(nrfx_err != NRFX_SUCCESS)
	{
		printk("Could not get SPI device\n");
		return;
	}
}

static void ADS1299_Platform_SpiDeInit(void)
{
	nrfx_spi_uninit(&spi_dev);
}

static void ADS1299_Platform_SpimInit(nrfx_spim_evt_handler_t spim_cb)
{
	nrfx_err_t nrfx_err;

	nrfx_err =  nrfx_spim_init(
		&spim_dev,
		&spim_config,
		spim_cb,
		NULL);
	
	if(nrfx_err != NRFX_SUCCESS)
	{
		printk("Could not get SPI device\n");
		return;
	}
}

static void ADS1299_Platform_SpimDeInit(void)
{
	nrfx_spim_uninit(&spim_dev);
}

void ADS1299_Platform_Hw_ConfigSpi(uint8_t spi_mode)
{
	IRQ_CONNECT(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 6, nrfx_isr,
		    nrfx_spim_1_irq_handler, 0);

	ADS1299_Platform_SpiDeInit();
	ADS1299_Platform_SpimDeInit();

	if(spi_mode == 2) // DMA
	{
		ADS1299_Platform_SpimInit(ADS1299_Platform_SpimEventCb);
	}
	else if(spi_mode == 1) // IT
	{
		ADS1299_Platform_SpiInit(ADS1299_Platform_SpiEventCb);
	}
	else if(spi_mode == 0) // Polling
	{
		ADS1299_Platform_SpiInit(NULL);
	}
}

void ADS1299_Platform_Hw_ConfigGpio(void)
{
	nrfx_err_t err;

	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(gpiote)),
	    DT_IRQ(DT_NODELABEL(gpiote), priority),
	    nrfx_isr, nrfx_gpiote_irq_handler, 0);

	// nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(0);

	// err = nrfx_gpiote_out_init(ADS1299O_RST, &out_config);
	// if (err != NRFX_SUCCESS) {
	// 	// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
	// 	return;
	// }

	// err = nrfx_gpiote_out_init(ADS1299O_START, &out_config);
	// if (err != NRFX_SUCCESS) {
	// 	// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
	// 	return;
	// }

	// err = nrfx_gpiote_out_init(ADS1299O_PWDN, &out_config);
	// if (err != NRFX_SUCCESS) {
	// 	// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
	// 	return;
	// }

	// err = nrfx_gpiote_out_init(ADS1299O_CS, &out_config);
	// if (err != NRFX_SUCCESS) {
	// 	// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
	// 	return;
	// }

	nrf_gpio_cfg_output(ADS1299O_RST);
	nrf_gpio_cfg_output(ADS1299O_START);
	nrf_gpio_cfg_output(ADS1299O_PWDN);
	nrf_gpio_cfg_output(ADS1299O_CS);

    nrf_gpio_pin_write(ADS1299O_CS, 0);
	nrf_gpio_pin_write(ADS1299O_START, 0);
	nrf_gpio_pin_write(ADS1299O_PWDN, 1);
	nrf_gpio_pin_write(ADS1299O_RST, 1);
	
	
	// nrfx_gpiote_out_set(ADS1299O_PWDN);
	// nrfx_gpiote_out_set(ADS1299O_RST);
}

static void ADS1299_Platform_Hw_DrdyEventCb(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	Std_ReturnType ret = E_OK;
	ADS1299_GetSensorData(0);
	//ret = CircularBuffer_Push(&ADS1299_Platform_EegCirBuf, &ADS1299_EEGRawDataBuffer[0][3], 24);
}

static void ADS1299_Platform_Hw_DrdyEventCbwDMA(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	Std_ReturnType ret = E_OK;
	ADS1299_GetSensorDataWithDMA(0);
	//ret = CircularBuffer_Push(&ADS1299_Platform_EegCirBuf, &ADS1299_EEGRawDataBuffer[0][3], 24);
}

void ADS1299_Platform_Hw_ConfigDrdy(uint8_t drdy_cb_mode)
{
	nrfx_err_t err;
	nrfx_gpiote_evt_handler_t drdy_it_cb;

	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_HITOLO,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
		.hi_accuracy = true,
		.skip_gpio_setup = false,
	};

	switch (drdy_cb_mode)
	{
	case 0:
	    drdy_it_cb = NULL;
		break;
	case 1:
	    drdy_it_cb = ADS1299_Platform_Hw_DrdyEventCb;
		break;
	case 2:
	    drdy_it_cb = ADS1299_Platform_Hw_DrdyEventCbwDMA;
		break;
	
	default:
		break;
	}

	/* Initialize input pin to generate event on high to low transition
	 * (falling edge) and call back
	 */
	err = nrfx_gpiote_in_init(ADS1299I_DRDY, &in_config, drdy_it_cb);
	if (err != NRFX_SUCCESS) {
		//LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_event_enable(ADS1299I_DRDY, true);
}


void ADS1299_Platform_Hw_ConfigDrdy_UsingPPI(void)
{
	nrfx_err_t err;
	nrfx_gpiote_evt_handler_t drdy_it_cb;

	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_HITOLO,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
		.hi_accuracy = true,
		.skip_gpio_setup = false,
	};

	/* Initialize input pin to generate event on high to low transition
	 * (falling edge) and call back
	 */
	err = nrfx_gpiote_in_init(ADS1299I_DRDY, &in_config, NULL);
	if (err != NRFX_SUCCESS) {
		//LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_event_enable(ADS1299I_DRDY, true);

	//
	// nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(0);

	// err = nrfx_gpiote_out_init(ADS1299O_CS, &out_config);
	// if (err != NRFX_SUCCESS) {
	// 	// LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
	// 	return;
	// }
	// nrfx_gpiote_out_set(ADS1299O_CS);	

	nrf_gpio_cfg_output(ADS1299O_CS);
    nrf_gpio_pin_write(ADS1299O_CS, 0);

	/* Setup SPIM */

	nrfx_spim_xfer_desc_t spim_buffer = {
		.p_tx_buffer = NULL,\
		.tx_length = 0,\
		.p_rx_buffer = &ADS1299_EEGRawDataBuffer[0][0],\
		.rx_length = 3 + ADS1299_NUMBER_OF_CHANNEL * ADS1299_BYTEDATA_RESOLUTION
	};

	err =  nrfx_spim_xfer(&spim_dev, &spim_buffer, NRFX_SPIM_FLAG_HOLD_XFER); 

	/* Allocate a (D)PPI channel. */
#if defined(DPPI_PRESENT)
	uint8_t channel;
	err = nrfx_dppi_channel_alloc(&channel);
#else
	nrf_ppi_channel_t channel;
	err = nrfx_ppi_channel_alloc(&channel);
#endif
	if (err != NRFX_SUCCESS) {
		//LOG_ERR("(D)PPI channel allocation error: %08x", err);
		return;
	}

	nrfx_gppi_channel_endpoints_setup(channel,
		nrf_gpiote_event_address_get(NRF_GPIOTE,
			nrfx_gpiote_in_event_get(ADS1299I_DRDY)),
		nrf_gpiote_task_address_get(NRF_SPIM1,
			nrfx_spim_start_task_get(&spim_dev)));
	
	/* Enable (D)PPI channel. */
#if defined(DPPI_PRESENT)
	err = nrfx_dppi_channel_enable(channel);
#else
	err = nrfx_ppi_channel_enable(channel);
#endif

}