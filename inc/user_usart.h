/*
 * user_usart.h
 *
 *  Created on: 2017. 4. 4.
 *      Author: netbugger
 */

#ifndef INC_USER_USART_H_
#define INC_USER_USART_H_

#include <stdio.h>

#if 1
#define LOG_DBG	printf
#else
#define LOG_DBG(...)
#endif

#if 1
#define LOG_NOR	printf
#else
#define LOG_NOR(...)
#endif

#if 1
#define LOG_ERR	printf
#else
#define LOG_ERR(...)
#endif

#define UART_RX_BUFFER_SIZE	128

typedef struct uart_rx_buffer_st {
	uint16_t tail;
	uint16_t head;
	uint16_t cnt;
	uint8_t buf[UART_RX_BUFFER_SIZE];
}uart_rx_buffer_t;

int _write(int file, char *ptr, int len);
int USART1_SEND(uint8_t *data, int len);
int get_usart_rx_data_blocking(uint8_t *buf, int size);



#endif /* INC_USER_USART_H_ */
