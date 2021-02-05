/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Bootloader         |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "com.h"

#define LED_DIR DDRD
#define LED_PRT PORTD
#define LED_BIT PD5


/* internet checksum */
static uint16_t checksum(uint8_t *data, size_t size)
{
    uint16_t *words = (uint16_t *) data;
    const size_t num_words = size / 2;
    uint32_t sum = 0;

    for (uint8_t i = 0; i < num_words; ++i)
        sum += words[i];

    /* fold the sum into 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return (uint16_t) sum;
}


static bool write_page(uint16_t *addr, const uint8_t* data, uint8_t size)
{
    const uint16_t page = *addr;
    uint8_t buffer[SPM_PAGESIZE];

    static_assert( SPM_PAGESIZE <= 255
                 , "use uint16_t for interation when pagesize is above 255" );

    if (size != (SPM_PAGESIZE + 2))
        return false;

    eeprom_busy_wait();

    /* clear page */
    boot_page_erase(page);

    uint8_t p = 1;
    uint16_t exp_chksum = data[p++];
    exp_chksum += (data[p++] << 8);

    for (uint8_t i = 0; i < SPM_PAGESIZE; ++i)
        buffer[i] = data[p++];

    if (exp_chksum != checksum(buffer, sizeof(buffer)))
        return false; /* failed */

    boot_spm_busy_wait();
    for (uint8_t i = 0; i < SPM_PAGESIZE; i += 2)
        boot_page_fill(page + i, buffer[i] + (buffer[i+1] << 8));

    boot_page_write(page);
    boot_spm_busy_wait();
    boot_rww_enable();

    /* increment address */
    *addr += SPM_PAGESIZE;

    return true; /* success */
}


static bool verify_device_id(const uint8_t* data, uint8_t size)
{
    return (size == 3)
        && (data[1] == SIGNATURE_0)
        && (data[2] == SIGNATURE_1)
        && (data[3] == SIGNATURE_2);
}


static void soft_reset(void)
{
    wdt_enable(WDTO_15MS);
    while(true);
}


int main(void)
{
    cli(); // disable all interrupts

    wdt_enable(WDTO_4S);
    LED_DIR |= _BV(LED_BIT);
    LED_PRT |= _BV(LED_BIT);


  /*  if (0xDD == boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS))
        while(1) { LED_PRT ^= _BV(LED_BIT); _delay_ms(200); }*/

    uart_init();
    uart_flush();

    uint16_t addr = 0x0;
    bool result = true;

    const uint8_t myid = read_id_from_EEPROM(127);
    uint8_t data[256];

    while(true)
    {
        uint8_t N = recv_packaged(data, myid);
        if (N > 0) // if valid data was read
        {
            switch (data[0])
            {
                case 'S': result = true; /* do nothing */        break;
                case 'I': result = verify_device_id (data, N-1); break;
                case 'W': result = write_page(&addr, data, N-1); break;
                case 'R': soft_reset();                          break;
                default : result = false; /* unknown cmd */      break;
            }
            // respond to host
            send_packaged((uint8_t*) (result? "Y":"N"), 1, myid);
            wdt_reset();
        }
    }

    return 0;
}

/* end sensorimotor bootloader */

