/*
 * printf_to_uart.c
 *
 *  Created on: Dec 27, 2024
 *      Author: dominik
 */

#include "usart.h"

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 1000);
	return ch;
}
