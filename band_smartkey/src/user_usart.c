/*
 * user_usart.c
 *
 *  Created on: 2017. 4. 4.
 *      Author: netbugger
 */

#include "em_usart.h"
#include "em_leuart.h"
#include "em_gpio.h"
#include "em_int.h"
#include "user_usart.h"
#include "InitDevice.h"

uart_rx_buffer_t gUartRxBuffer;
//uart_rx_buffer_t gLeUartRxBuffer;
#if 1
int _write(int file, char *ptr, int len)
{
  /* Implement your write code here, this is used by puts and printf for example */
  int i=0;
  for(i=0 ; i<len ; i++) {
	  if(*ptr == '\n') {
#ifdef LEUART_DEBUG
		  LEUART_Tx(LEUART0, '\r');
#else
		  USART_Tx(USART0, '\r');
#endif
	  }
#ifdef LEUART_DEBUG
	  LEUART_Tx(LEUART0, (*ptr++));
#else
	  USART_Tx(USART0, (*ptr++));
#endif
  }
  return len;
}
#endif

void init_dbg_to_leuart()
{
#ifdef LEUART_DEBUG
	/* Init from InitDevice.c */
	// $[LEUART0 initialization]
	LEUART_Init_TypeDef initleuart = LEUART_INIT_DEFAULT;

	initleuart.enable = leuartEnable;
	initleuart.baudrate = 9600;
	initleuart.databits = leuartDatabits8;
	initleuart.parity = leuartNoParity;
	initleuart.stopbits = leuartStopbits1;
	LEUART_Init(LEUART0, &initleuart);

	/* Configuring non-standard properties */
	LEUART_TxDmaInEM2Enable(LEUART0, 0);
	LEUART_RxDmaInEM2Enable(LEUART0, 0);

	/* Pin PF0 is configured to Push-pull */
	GPIO->P[5].MODEL = (GPIO->P[5].MODEL & ~_GPIO_P_MODEL_MODE0_MASK)
					| GPIO_P_MODEL_MODE0_PUSHPULL;

	/* Pin PF1 is configured to Input enabled */
	GPIO->P[5].MODEL = (GPIO->P[5].MODEL & ~_GPIO_P_MODEL_MODE1_MASK)
					| GPIO_P_MODEL_MODE1_INPUT;

	/* Module LEUART0 is configured to location 3 */
	LEUART0->ROUTE = (LEUART0->ROUTE & ~_LEUART_ROUTE_LOCATION_MASK)
					| LEUART_ROUTE_LOCATION_LOC3;

	/* Enable signals RX, TX */
	LEUART0->ROUTE |= LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN;
#endif
}


#if 0
uint8_t flag = 0;
#endif

int USART1_SEND(uint8_t *data, int len)
{
	int i=0;
	for(i=0; i<len; i++) {
		USART_Tx(USART1, *(data+i));
	}
	return len;
}

void USART0_RX_IRQHandler(void)
{

}
void USART1_RX_IRQHandler(void)
{
	uint8_t ch;

	uart_rx_buffer_t *pBuf = &gUartRxBuffer;

	/* Store Data */
	GPIO_PinOutToggle(PE12_PORT, PE12_PIN);
	ch = USART_Rx(USART1);

	if (pBuf->cnt < UART_RX_BUFFER_SIZE) {
		pBuf->buf[pBuf->tail] = (uint8_t) ch;
		pBuf->tail++;
		pBuf->cnt++;
		if (pBuf->tail >= UART_RX_BUFFER_SIZE) {
			pBuf->tail = 0;
		}
	} else {
		// Buffer Full, Reset?
	}
}

void LEUART0_IRQHandler(void)
{
#ifdef LEUART_DEBUG
	uint8_t ch;

	uart_rx_buffer_t *pBuf = &gLeUartRxBuffer;

	/* Store Data */
	ch = LEUART_Rx(LEUART0);

	if (pBuf->cnt < UART_RX_BUFFER_SIZE) {
		pBuf->buf[pBuf->tail] = (uint8_t) ch;
		pBuf->tail++;
		pBuf->cnt++;
		if (pBuf->tail >= UART_RX_BUFFER_SIZE) {
			pBuf->tail = 0;
		}
	} else {
		// Buffer Full, Reset?
	}
#endif
}

int peep_uart_rx()
{
	return gUartRxBuffer.cnt;
}

void init_uart_rx_buffer()
{
	gUartRxBuffer.cnt = 0;
	gUartRxBuffer.head = 0;
	gUartRxBuffer.tail = 0;
#ifdef LEUART_DEBUG
	gLeUartRxBuffer.cnt = 0;
	gLeUartRxBuffer.head = 0;
	gLeUartRxBuffer.tail = 0;
#endif
}

int get_usart_rx_data(uint8_t *buf, int size)
{
	int i, cpcnt;

	INT_Disable();

	if (gUartRxBuffer.cnt >= size) {
		cpcnt = size;
	} else {
		cpcnt = gUartRxBuffer.cnt;
	}

	for (i = 0; i < cpcnt; i++) {
		buf[i] = gUartRxBuffer.buf[gUartRxBuffer.head];
		gUartRxBuffer.cnt--;
		gUartRxBuffer.head++;
		if (gUartRxBuffer.head >= UART_RX_BUFFER_SIZE) {
			gUartRxBuffer.head = 0;
		}
	}

	INT_Enable();
	return cpcnt;
}

int get_usart_rx_data_blocking(uint8_t *buf, int size)
{
	int i, cpcnt;

	while(gUartRxBuffer.cnt < size) {
		Delay(10);
	}
	INT_Disable();

	cpcnt = size;

	for (i = 0; i < cpcnt; i++) {
		buf[i] = gUartRxBuffer.buf[gUartRxBuffer.head];
		gUartRxBuffer.cnt--;
		gUartRxBuffer.head++;
		if (gUartRxBuffer.head >= UART_RX_BUFFER_SIZE) {
			gUartRxBuffer.head = 0;
		}
	}

	INT_Enable();
	return cpcnt;
}

#ifdef LEUART_DEBUG
int get_leuart_rx_data(uint8_t *buf, int size)
{
	int i, cpcnt;

	INT_Disable();

	if (gLeUartRxBuffer.cnt >= size) {
		cpcnt = size;
	} else {
		cpcnt = gLeUartRxBuffer.cnt;
	}

	for (i = 0; i < cpcnt; i++) {
		buf[i] = gLeUartRxBuffer.buf[gLeUartRxBuffer.head];
		gLeUartRxBuffer.cnt--;
		gLeUartRxBuffer.head++;
		if (gLeUartRxBuffer.head >= UART_RX_BUFFER_SIZE) {
			gLeUartRxBuffer.head = 0;
		}
	}

	INT_Enable();
	return cpcnt;
}
#endif

void init_user_usart()
{
	/* USART1[BTIF] Enable RX Interrupt */
	USART_IntClear(USART1, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART1_RX_IRQn);
	USART_IntEnable(USART1, USART_IEN_RXDATAV);
	NVIC_EnableIRQ(USART1_RX_IRQn);

#if 0
	/* USART0[CONSOLE] Rx Interrupt */
	USART_IntClear(USART0, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART0_RX_IRQn);
	USART_IntEnable(USART0, USART_IEN_RXDATAV);
	NVIC_EnableIRQ(USART0_RX_IRQn);
#endif

	init_uart_rx_buffer();


#ifdef LEUART_DEBUG
	/* Enable LEUART RX Interrupt */
	LEUART_IntClear(LEUART0, LEUART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(LEUART0_IRQn);
	USART_IntEnable(USART0, USART_IEN_RXDATAV);
	NVIC_EnableIRQ(LEUART0_IRQn);
	init_usart_rx_buffer();
#endif
}
