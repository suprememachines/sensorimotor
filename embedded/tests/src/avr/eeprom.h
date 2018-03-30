
#include <stdio.h>

typedef unsigned char uint8_t;

uint8_t motor_id = 23; //TODO make array


void eeprom_busy_wait(void) {}

uint8_t eeprom_read_byte(uint8_t* ) {
//	printf("rd motor id: %u\n", motor_id & 0x7F);
	return motor_id;
}

void eeprom_write_byte(uint8_t*, uint8_t b) {
	motor_id = b;
//	printf("wr motor id: %u\n", motor_id & 0x7F);
}

void set_motor_id(uint8_t id) { motor_id = id; }

