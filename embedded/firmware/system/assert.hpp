/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
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
	led::red::reset();
	led::yellow::reset();
}


void assert(bool condition, uint8_t code = 0) {
	if (condition) return;
	led::red::reset();
	led::yellow::reset();
	while(1) {
		blink(code);
		xpcc::delayMilliseconds(1000);
	}
}

} /* namespace supreme */

#endif /* SUPREME_COMMON_ASSERT_HPP */
