/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines Sensorimotor   |
 | Firmware for Board-Revision 1.2 |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

#ifndef SUPREME_EEPROM_HPP
#define SUPREME_EEPROM_HPP

#include <avr/eeprom.h>
#include "constants.hpp"


namespace supreme {
namespace EEPROM {

	enum address {
		id   = 0,
		dir  = 1,
	};

	inline uint8_t read(const size_t addr) {
		eeprom_busy_wait();
		return eeprom_read_byte((uint8_t*)addr);
	}

	inline void update(const size_t addr, uint8_t val) {
		eeprom_busy_wait();
		eeprom_update_byte((uint8_t*)addr, val);
	}

	inline void read_id(uint8_t& motor_id) {
		uint8_t read_id = read(address::id);
		if (read_id) /* if MSB is set, this id was written before */
			motor_id = read_id & constants::max_id; // only values 0..127 allowed
	}

	inline bool read_dir(void) {
		uint8_t dir = read(address::dir);
		return static_cast<bool>(dir & 0x1); // avoid reading garbage
	}

	inline void read_dir (bool& default_dir) { default_dir = read(address::dir); }

	inline void update_id (uint8_t new_id  ) { update(address::id , new_id | 0x80); } // set MSB to signal that id was initally written
	inline void update_dir(bool default_dir) { update(address::dir, default_dir & 0x1); }

} /* namespace EEPROM */
} /* namespace supreme */

#endif /* SUPREME_EEPROM_HPP */
