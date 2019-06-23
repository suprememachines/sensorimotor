/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_MOTOR_IFX9201SG_HPP
#define SUPREME_MOTOR_IFX9201SG_HPP

#include <common/bitscale.hpp>

namespace supreme {

class motor_ifx9201sg {
public:

	motor_ifx9201sg()
	{
		/* setup motor bridge */
		disable();           // motor bride disabled by default
		motor::VSO::set();   // enable motor bridge logic
		motor::DIR::set();   // set direction
		motor::PWM::reset(); // disable pwm

		/*             f_clock
		    f_pwm = -------------- = 15.625 kHz
		            N x (1 + 1023)
		*/
		TCCR1A = (1<<COM1A1) // OC1A and OC1B are cleared on compare match
		       | (1<<WGM10)  // Wave Form Generation is Fast PWM 10 Bit, top is 0x3ff (1023)
		       | (1<<WGM11);
		TCCR1B = (1<<WGM12)
		       | (1<<CS10);  // set prescaler N = 1


		OCR1A = 0; // set pwm to zero duty cycle


		/* Idea: set up a second pwm on the disable pin and synchronize the duty_cycle_shares.
		 *       test if disabling the h-bridge does change the motors behavior, in order to
		 *       utilize this as a second mode.
		 *
		 *    +----+          +----+
		 *    | on | freerun  | on | freerun
		 *    +    +----------+    +----------+
		 *              +-----+         +-----+
		 *              | dis |         | dis |     can this be used to break?
		 *    ----------+     +---------+     +-----
		 */
	}

	void toggle_direction() { motor::DIR::toggle(); }
	void set_dir(bool dir) {
		if (dir) motor::DIR::set();
		else motor::DIR::reset();
	}

	/* TODO set min and max pwm */
	void set_pwm(uint8_t dc) { OCR1A = promote_N<2>(dc); }

	void enable() { motor::DIS::reset(); }
	void disable() { motor::DIS::set(); }
};

using motordriver_t = motor_ifx9201sg;

} /* namespace supreme */

#endif /* SUPREME_MOTOR_IFX9201SG_HPP */
