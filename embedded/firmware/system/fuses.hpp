/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 15th 2021               |
 +---------------------------------*/

#ifndef SUPREME_FUSES_HPP
#define SUPREME_FUSES_HPP

#include <avr/boot.h>

namespace supreme {

#define _GET_L_FUSES() boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS)
#define _GET_H_FUSES() boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS)
#define _GET_E_FUSES() boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS)

bool check_fuses(uint8_t lf, uint8_t hf, uint8_t ef)
{
	return ( lf == _GET_L_FUSES())
	   and ( hf == _GET_H_FUSES())
	   and ( ef == _GET_E_FUSES());
}

} /* namespace supreme */

#endif /* SUPREME_FUSES_HPP */
