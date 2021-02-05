/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Bootloader         |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

#ifndef SUPREME_COM_H
#define SUPREME_COM_H

#include <avr/eeprom.h>
#include "uart.h"

/* wrap/unwrap the data to be sent and received into a valid
   raw data package that can be safely processed (or ignored)
   by sensorimotors. */
void send_packaged(const uint8_t* data, uint8_t size, uint8_t myid);
uint8_t recv_packaged(uint8_t* data, uint8_t myid);


/* read id from EEPROM, return default if ID was not yet set */
uint8_t read_id_from_EEPROM(uint8_t default_id);

#endif /* SUPREME_COM_H */

