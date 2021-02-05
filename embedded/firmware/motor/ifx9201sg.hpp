/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 2021                    |
 +---------------------------------*/

#ifndef SUPREME_MOTOR_IFX9201SG_HPP
#define SUPREME_MOTOR_IFX9201SG_HPP

#include <common/bitscale.hpp>
#include <xpcc/architecture/platform.hpp>

/*
	TODO: we need to go higher in frequency,
	15.625 kHz is still audible by children and pets.

	human   20 kHz

	how to handle pets?
	rabbit  42 kHz
	dog     45 kHz
	cat     64 kHz
	rat     76 kHz
	mouse   91 kHz

	Using FAST PWM mode 14:
	we need to set the TOP value

	fclock = 16.000.000 Hz
	prescaler N = 1

	user provided pwm value: x = 0..255
	TOP = 799 = 800 - 1

	How to scale from pwm to TOP?
	Use fix integer scaling of K = 3

	fPWM = fclock / (TOP+1)*N

	PWM user values 0..255 (min-max)

	variants:
	  255 * 3     = 765           -> 20.90 KHz    *3                (5% over-spec, most efficient calc)
	  255 * 3.125 = 797 (796.875) -> 20.07 kHz    *25, >> 3 => 25/8 (slightly over-spec in datasheet)
	  255 * 3.25  = 829 (828.750) -> 19.30 kHz    *13, >> 2 => 13/4 (best compromise)

	alternative: use factor 3 and discard the offset as motor friction compensation
	TOP = 255 * 3 + 35 (offset)

	currently: 16 MHz / 1024 = 15.6 KHz (still in kids' audible range)
*/

namespace supreme {

class motor_ifx9201sg {

	/*            f_clock
	   f_pwm = -------------- = 15.625 kHz
	           N x (1 + 1023)
	*/

	static constexpr uint8_t tccr1a = (1<<COM1A1) // OC1A is cleared on compare match, set at bottom, non-inverting mode
	                                | (1<<WGM10)  // Wave Form Generation is Fast PWM, 10 Bit, top is 0x3ff (1023)
	                                | (1<<WGM11); // we're using 10bit timer to reach 15.625 kHz
	static constexpr uint8_t tccr1b = (1<<WGM12)
	                                | (1<<CS10);  // set prescaler N = 1
public:

	motor_ifx9201sg()
	{
		/* setup motor bridge */
		motor::DIS::set();   // motor bride disabled by default
		motor::PWM::reset(); // disable pwm pin
		motor::VSO::set();   // enable motor bridge logic
		motor::DIR::set();   // set direction

		/* configure timer 1 for PWM mode */
		TCCR1A = tccr1a;
		TCCR1B = tccr1b;

		OCR1A = 0;
	}

	void set_dir(bool dir) {
		if (dir) motor::DIR::set();
		else motor::DIR::reset();
	}

	void set_pwm(uint8_t dc) {
		if (dc != 0) {
			motor::DIS::reset();
			OCR1A = promote_N<2>(dc);
		}
		else {
			motor::DIS::set();
			OCR1A = 0; // set pwm to zero duty cycle
		}
	}
};

using motordriver_t = motor_ifx9201sg;

} /* namespace supreme */


/* Prob: The used H-bridge has free-wheeling only in the current set direction (DIR).
 * Idea: Set up a second pwm on the disable pin and have DIS on inverted duty cycles.
 *       This can be used to make free-wheeling adjustable.
 * Note: However, this does not yet work for the IFX9201 integrated H-bridge at 15.625 kHz.
 *       The period time for one cycle at given frequency is only 64µs.
 *       Enable/Disable delay is given with 80µs (from datasheet).
 *       Need another idea.
 *
 *    +----+          +----+
 *    | on | freerun  | on | freerun
 *    +    +----------+    +----------+
 *              +-----+         +-----+
 *              | dis |         | dis |     could be used to fine-tune breaking.
 *    ----------+     +---------+     +-----
 */

#endif /* SUPREME_MOTOR_IFX9201SG_HPP */
