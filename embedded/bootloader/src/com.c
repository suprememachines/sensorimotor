/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Bootloader         |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

/*
    common code for bootloader and example firmware
    e.g. uart data packaging and eeprom ID reading
*/

#include "com.h"

void send_packaged(const uint8_t* data, uint8_t size, uint8_t myid)
{
    uint8_t cs = 0x54 + myid + size;

    uart_putc(0xff); // sync 0
    uart_putc(0xff); // sync 1
    uart_putc(0x56); // raw data client reponse code
    uart_putc(myid); // board identifier
    uart_putc(size); // number of bytes to be sent

    for (uint8_t i = 0; i < size; ++i) {
        uart_putc(data[i]);
        cs += data[i];
    }
    cs = ~cs + 1; /* simple two's complement checksum */
    uart_putc(cs); // add checksum
}


uint8_t recv_packaged(uint8_t* data, uint8_t myid)
{
    uint8_t size = 0;
    while (true) {
        uint8_t cs = 0x53 + myid;
        if (0xff != uart_getc()) break; // sync 0
        if (0xff != uart_getc()) break; // sync 1
        if (0x55 != uart_getc()) break; // raw data host send code
        if (myid != uart_getc()) break; // board identifier
        size = uart_getc(); // number of bytes to be received
        cs += size;
        for (uint8_t i = 0; i < size; ++i) {
            data[i] = uart_getc();
            cs += data[i];
        }
        cs += uart_getc(); // get recv checksum
        if (cs == 0) break; // received valid package?
    }
    return size;
}


/* only ID 0..127 are valid, IDs are saved with MSB set. ID | 0x80 */
uint8_t read_id_from_EEPROM(uint8_t default_id) {
    eeprom_busy_wait();
    uint8_t id = eeprom_read_byte((uint8_t*)0);
    /* check MSB is set to verify that the id was written before */
    return id ? (id & 0x7F) : default_id;
}

