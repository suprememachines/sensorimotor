/*-----------------------------------+
 | Jetpack Cognition Lab, Inc.       |
 | Supreme Machines Sensorimotor     |
 | Simple UART Lib for Bootloader    |
 | Author: Matthias Kubisch          |
 | kubisch@informatik.hu-berlin.de   |
 | Last Update: January 2021         |
 +-----------------------------------*/

#ifndef SUPREME_UART_H
#define SUPREME_UART_H

#include <stdint.h>
#include <stdbool.h>

bool byte_received(void);
uint8_t uart_getc(void);
void uart_putc(uint8_t);
void uart_init(void);
void uart_flush(void);

#endif /* SUPREME_UART_H */
