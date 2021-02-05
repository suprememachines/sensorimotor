/*-----------------------------------+
 | Jetpack Cognition Lab, Inc.       |
 | Supreme Machines Sensorimotor     |
 | Simple UART Lib for Bootloader    |
 | Author: Matthias Kubisch          |
 | kubisch@informatik.hu-berlin.de   |
 | Last Update: January 2021         |
 +-----------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

#ifndef F_CPU
    #error "F_CPU undefined, please define in Makefile"
#endif

#ifndef BAUD_RATE
    #error "BAUD_RATE undefined, please define in Makefile"
#endif

#define WAITBYTE_US (10000000UL/BAUD_RATE)

#ifdef SENSORIMOTOR_REV1_1
    #define DE_DIR DDRD
    #define DE_PRT PORTD
    #define DE_BIT PD3
    #define RD_DIR DDRC
    #define RD_PRT PORTC
    #define RD_BIT PC0
#elif SENSORIMOTOR_REV1_2
    #define DE_DIR DDRD
    #define DE_PRT PORTD
    #define DE_BIT PD2
    #define RD_DIR DDRC
    #define RD_PRT PORTC
    #define RD_BIT PC0
#else
    #error "Unspecified device, please define in Makefile"
#endif

bool byte_received(void) {
    return (UCSR0A & (1<< RXC0));
}


uint8_t uart_getc(void) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void uart_flush(void) {
    static uint8_t dat = 0;
    while(byte_received())
        dat += uart_getc();
}

void uart_putc(uint8_t c) {
    /* send buffer ready to send a char */
    loop_until_bit_is_set(UCSR0A, UDRE0);

    /* enable RS485 send_mode */
    DE_PRT |= _BV(DE_BIT);    // set 'Drive Enable' to high
    _delay_us(WAITBYTE_US); // wait 1 byte for the transceiver

    UDR0 = c;

    /* loopback method, wait for the character
       to be received before reenabling recv mode */
    uart_getc();

    /* enable RS485 recv_mode */
    DE_PRT &= ~_BV(DE_BIT); // set DE to low
}

void uart_init(void) {
    UCSR0A = (1 << U2X0);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
    UBRR0L = (uint8_t)((F_CPU + BAUD_RATE * 4L)/(BAUD_RATE * 8L) - 1);

    DE_DIR |= _BV(DE_BIT); // set drive enable to output
    RD_DIR |= _BV(RD_BIT); // set read disable to output
    RD_PRT &= ~_BV(RD_BIT); // set read disable to low
}

