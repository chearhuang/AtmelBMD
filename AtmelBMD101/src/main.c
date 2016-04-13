/**
 * \file
 *
 * \brief Getting Started Application.
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage Getting Started Application
 *
 * \section Purpose
 *
 * The Getting Started example will help new users get familiar with Atmel's
 * SAM family of microcontrollers. This basic application shows the startup
 * sequence of a chip and how to use its core peripherals.
 *
 * \section Requirements
 *
 * This application has been tested on following boards:
 * - SAM D21/R21/L21/L22/C21 Xplained Pro
 * - SAM D10 Xplained Mini
 *
 * \section Description
 *
 * The program demo how LED,button,delay,interrupt and timer/counter work .
 *
 * \section Usage
 *
 * -# Build the program and download it inside the evaluation board.
 * -# On the computer, open and configure a terminal application
 *    (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *   - 38400 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 * -# Start the application.
 * -# The LED(s) should start blinking on the board. In the terminal window, the
 *    following text should appear (values depend on the board and chip used):
 *    \code
 *     -- Getting Started Example xxx --
 *     -- xxxxxx-xx
 *     -- Compiled: xxx xx xxxx xx:xx:xx --
 *    \endcode
 * -# Pressing and release button SW0 should make LED0 on and off
 *    blinking.
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "asf.h"
#include "stdio_serial.h"
#include "conf_uart_serial.h"
#include "BMD101.h"

#ifdef __cplusplus
extern "C" {
#endif

struct usart_module ext3_uart_module;
uint8_t *tmp_data;
extern signed int Get_PoorSignal_HeartRate(void);
/**
 *  Configure UART console.
 */
static void configure_console(void)
{
	struct usart_config usart_conf;
	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = EXT3_UART_SERCOM_MUX_SETTING ;
	usart_conf.pinmux_pad0 = EXT3_UART_SERCOM_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = EXT3_UART_SERCOM_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = EXT3_UART_SERCOM_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = EXT3_UART_SERCOM_PINMUX_PAD3;
	usart_conf.baudrate    = 57600;
	stdio_serial_init(&ext3_uart_module, EXT3_UART_MODULE, &usart_conf);	
	usart_enable(&ext3_uart_module);
}

	
int main(void)
{
	struct port_config pin;

	volatile uint8_t raw_data=0;
	system_init();
	/*Configure UART console.*/
    configure_console();
	/* Output example information */
    /*Configures PORT for LED0*/
	port_get_config_defaults(&pin);
	pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED0_PIN, &pin);
	port_pin_set_output_level(LED0_PIN, LED0_INACTIVE);
	/*main loop*/
	while(1)
	{ 
		raw_data = Get_PoorSignal_HeartRate();
		
		/***************printf the raw data**************/
		printf("\nRaw_data is %d\n",raw_data);
			
	}
	
}

#ifdef __cplusplus
}
#endif
