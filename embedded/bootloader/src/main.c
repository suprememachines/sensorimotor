/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Example Firmware   |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

/*
  Note: This is example code for alternative firmware developments.
  Any alternative sensorimotor firmware must respond to UART bytes
  sequences such as [0xFF 0xFF 0xC0 myid 0x01 'S' checksum] in order
  to enter the bootloader.
  The bootloader is not starting on normal startup, but the firmware
  should jump to bootloader on external UART request (by the host).
  Board variants with a reset button (e.g. Rev.1.1) should jump to the
  bootloader on reset-button pressed, but nothing else. After normal
  power-up (or brownout) the firmware should start immediately for
  not wasting any time and continue the sensorimotor loop right away.
  We're using RS485 half-duplex, don't send any bytes unless requested
  and don't forget to enable the receiver and only enable the driver
  when responding. Disable transmitter after sending only using the
  loopback method.

  Checklist for any new firmware:
  [ ] disable watchdog timer, right on startup
  [ ] listen to UART at 1 MBaud and receive byte sequences
  [ ] don't send any bytes unless requested by host
  [ ] keep driver disabled unless sending responses
  [ ] jump to bootloader location on receiving start sequence
  [x] Have fun programming :)
*/


#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "uart.h"
#include "com.h"

#define LED_DIR DDRD
#define LED_PRT PORTD
#define YEL_BIT PD5
#define RED_BIT PD7

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void) {
    MCUSR &= ~(_BV(WDRF));
    wdt_disable();
}

typedef void (*jump_t)(void);

static void jump(uint16_t addr)
{
    jump_t func = (jump_t) addr;
    func();
}

int main(void)
{
    /* jump to bootloader on reset-button pressed (only rev 1.1) */
    if (bit_is_set(MCUSR, EXTRF)) {
        MCUSR &= ~(_BV(EXTRF));
        jump(0x3e00);
    }

    /* init yellow LED */
    LED_DIR |= _BV(YEL_BIT);

    uart_init();
    uint8_t data[256];
    uint8_t myid = read_id_from_EEPROM(127);

    unsigned k = 0;
    while (1)
    {
        if (k++ >= 25000) {
            LED_PRT ^= _BV(YEL_BIT);
            k = 0;
        }

        if (byte_received()) {
            uint8_t N = recv_packaged(data, myid);
            if (N > 0 && 'S' == data[0])
                break;
        } else
            _delay_us(10);
    }

    /* turn off LEDs */
    LED_PRT &= ~(_BV(RED_BIT));
    LED_PRT &= ~(_BV(YEL_BIT));

    /* send response */
    send_packaged((uint8_t*) "Y", 1, myid);

    /* enter bootloader */
    jump(0x3e00);

    return 0;
}
