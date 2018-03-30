/*

   +----------------------------+
   | 2017, Supreme Machines GbR |
   | Sensorimotor Firmware      |
   +----------------------------+

*/

#include <xpcc/architecture/platform.hpp>
#include <sensorimotor_core.hpp>
#include <communication.hpp>
#include <adc.hpp>

/* this is called once TCNT0 = OCR0A = 249 *
 * resulting in a 1 ms cycle time, 1kHz    */
volatile bool current_state = false;
ISR (TIMER0_COMPA_vect)
{
	current_state = !current_state;
}


int main()
{
	Board::initialize();
	led::yellow::setOutput();
	led::red::setOutput();

	supreme::adc::init();

	typedef supreme::sensorimotor_core core_t;
	core_t ux;

	/* Design of the 1kHz main loop:
	 * 16Mhz clock, prescaler 64 -> 16.000.000 / 64 = 250.000 increments per second
	 * diveded by 1000 -> 250 increments per ms
	 * hence, timer compare register to 250-1 -> ISR inc ms counter -> 1kHz loop
	 *
	 * configure timer 0:
	 */
	TCCR0A = (1<<WGM01);             // CTC mode
	TCCR0B = (1<<CS01) | (1<<CS00);  // set prescaler to 64
	OCR0A = 249;                     // set timer compare register to 250-1
	TIMSK0 = (1<<OCIE0A);            // enable compare interrupt

	unsigned long cycles = 0;

	supreme::communication_ctrl<core_t> com(ux);

	bool previous_state = false;

	while(1) /* main loop */
	{
		if (current_state != previous_state) {
			led::red::set();   // red led on, begin of cycle
			ux.step();
			supreme::adc::restart();
			++cycles;
			led::red::reset(); // red led off, end of cycle
			previous_state = current_state;
		}
		com.step();
	}
	return 0;
}
