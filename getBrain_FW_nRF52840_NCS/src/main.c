/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */



#include "../getBrain_Apps/App_CollectStream.h"

void main(void)
{
	App_CollectStream_BLE_MainRun();
	
	while(1)
	{

	}
	
}




// #include <zephyr.h>
// #include <sys/printk.h>
// #include <nrfx_spim.h>


// static const nrfx_spim_t spim_dev = NRFX_SPIM_INSTANCE(1);

// /**< Flag used to indicate that SPI instance completed the transfer. */
// static volatile bool spi_xfer_done = false; 

// #define TEST_STRING "Nordic123456789012345678901234567890"
// static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
// static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];  /**< RX buffer. */
// static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

// void spim_event_handler(nrfx_spim_evt_t const * p_event,
//                        void *                  p_context)
// {
//     spi_xfer_done = true;
// 	printk("Transfer completed.\n");
// }

// void main()
// {
// 	// Init
// 	nrfx_err_t nrfx_err;

//     nrfx_spim_config_t spim_config = {
//     	.sck_pin = 13,
//     	.mosi_pin = 33,
//     	.miso_pin = 40,
//     	.ss_pin   = 34,
//     	.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
//     	.orc          = 0xFF,
//     	.frequency    = NRF_SPIM_FREQ_1M,
//     	.mode         = NRF_SPIM_MODE_1,
//     	.bit_order    = NRF_SPIM_BIT_ORDER_MSB_FIRST,
//     	.miso_pull    = NRF_GPIO_PIN_NOPULL,   
// 		.ss_active_high = false,
//     };

// 	IRQ_CONNECT(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 6, nrfx_isr,
// 		    nrfx_spim_1_irq_handler, 0);

//     nrfx_err = nrfx_spim_init(&spim_dev, &spim_config, spim_event_handler, NULL);
// 	if(nrfx_err != NRFX_SUCCESS)
// 	{
// 		printk("SPIM Init Error\n");
// 	}

// 	nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(m_tx_buf, m_length, m_rx_buf, m_length);

// 	while(1)
// 	{
// 		// Reset rx buffer and transfer done flag
//         memset(m_rx_buf, 0, m_length);
//         spi_xfer_done = false;

// 		nrfx_spim_xfer(&spim_dev, &xfer_desc, 0);


//         while (!spi_xfer_done)
//         {
//             __WFE();
//         }
// 	}
// }
