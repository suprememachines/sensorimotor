/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines Sensorimotor   |
 | Firmware for Board-Revision 1.1 |
 | Author: Matthias Kubisch        |
 | kubisch@informatik.hu-berlin.de |
 | Last Update: January 2021       |
 +---------------------------------*/

#ifndef SUPREME_CONSTANTS_HPP
#define SUPREME_CONSTANTS_HPP

namespace supreme {

namespace constants {

	constexpr uint8_t max_id = 0x7F;
	constexpr uint8_t syncbyte = 0xFF;

	constexpr uint8_t LF = 0xCF;
	constexpr uint8_t HF = 0xDD;
	constexpr uint8_t EF = 0xFD;

	//TODO define visible blink patterns
	enum class assertion {
		process_command       =  2,
		waiting_for_id        =  3,
		waiting_for_data      =  4,
		eating_others_data    =  5,
		buffer_not_exceeded   =  8,
		unknown_command_state = 17,
		single_byte_commands  = 77,
		no_sync_in_finished   = 55,
		wrong_fuse_bits_set   = 0xCC,
	};


} /* namespace constants */

} /* namespace supreme */

#endif /* SUPREME_CONSTANTS_HPP */
