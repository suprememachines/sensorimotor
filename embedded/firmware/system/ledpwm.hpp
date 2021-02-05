/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | April 1 2020                    |
 +---------------------------------*/

#ifndef SUPREME_SYSTEM_LED_HPP
#define SUPREME_SYSTEM_LED_HPP

#include <xpcc/architecture/platform.hpp>

namespace supreme {

/* OC0B is PD5 (LED YELLOW) */

class Pulsed_LED {

	static constexpr uint8_t tccr0a = (1<<COM0B1) // Set timer0 on compare match mode, set OC0B at BOTTOM, non-inverting mode
	                                | (1<<WGM00)  // Wave Form Generation is Fast PWM, 8 Bit, top is 0xff (255)
	                                | (1<<WGM01);
	static constexpr uint8_t tccr0b = (1<<CS00);  // set prescaler N = 1

	uint16_t osc = 0;
	bool rec = true;
public:

	void enable(void) {
		TCCR0A = tccr0a;
		TCCR0B = tccr0b;
		OCR0B = 0;
	}

	void disable(void) {
		TCCR0A = 0;
		TCCR0B = 0;
	}

	void set_pwm(uint8_t dc) { OCR0B = dc; }

	void idle(void) {
		if (rec) ++osc; else --osc;
		if (2047==osc) rec = false;
		else if(osc == 0) rec = true;
		set_pwm(osc>>6);
	}

	void reset() { osc = 0; rec = true; }
};

} /* namespace supreme */

#endif /* SUPREME_SYSTEM_LED_HPP */
