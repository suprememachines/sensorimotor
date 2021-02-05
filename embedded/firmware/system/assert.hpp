/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Rev 1.1 Firmware   |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 2021                    |
 +---------------------------------*/

#ifndef SUPREME_COMMON_ASSERT_HPP
#define SUPREME_COMMON_ASSERT_HPP

#include <xpcc/architecture/platform.hpp>

namespace supreme {

void blink(uint8_t code) {
	for (uint8_t i = 0; i < 8; ++i)
	{
		if ((code & (0x1 << i)) == 0)
		{
			led::red::reset();
			led::yellow::set();
		} else {
			led::red::set();
			led::yellow::reset();
		}
		xpcc::delayMilliseconds(250);
		led::red::reset();
		led::yellow::reset();
		xpcc::delayMilliseconds(250);
	}
}

void assert(bool condition, constants::assertion code) {
	if (condition) return;
	//TODO halt PWMs and other stuff

	led::red::reset();
	led::yellow::reset();
	while(1) {
		blink(static_cast<uint8_t>(code));
		xpcc::delayMilliseconds(1000);
	}
}

inline void stop(constants::assertion code) { assert(false, code); }

} /* namespace supreme */

#endif /* SUPREME_COMMON_ASSERT_HPP */
