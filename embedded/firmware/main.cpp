/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 2021                    |
 +---------------------------------*/

#include <avr/wdt.h>
#include <xpcc/architecture/platform.hpp>
#include <boards/sensorimotor_rev1_1.hpp>
#include <system/core.hpp>
#include <system/communication.hpp>
#include <system/adc.hpp>
#include <system/fuses.hpp>
#include <system/bootloader.hpp>
#include <external/i2c_sensor.hpp>

/* Disable all previous watchdog timer activities of the
   bootloader, in order to not interfere with the firmware.*/
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void) {
	MCUSR &= ~(_BV(WDRF));
	wdt_disable();
}

/* this is called once TCNT2 = OCR2A = 249 *
 * resulting in a 1 ms cycle time, 1kHz    */
volatile bool current_state = false;
ISR (TIMER2_COMPA_vect) {
	current_state = !current_state;
	xpcc::Clock::increment();
}

int main()
{
	using namespace supreme;

	/* jump to bootloader on reset-button pressed (only rev 1.1) */
	if (bit_is_set(MCUSR, EXTRF)) {
		MCUSR &= ~(_BV(EXTRF));
		start_bootloader();
	}

	Board::initialize();

	if (not check_fuses(constants::LF, constants::HF, constants::EF))
		stop(constants::assertion::wrong_fuse_bits_set);

	supreme::adc::init();
	supreme::adc::restart();

	typedef sensorimotor_core<supreme::motordriver_t> core_t;
	typedef ExternalSensor                            exts_t;
	core_t core;
	exts_t exts;

	/* Design of the 1kHz main loop:
	 * 16Mhz clock, prescaler 64 -> 16.000.000 / 64 = 250.000 increments per second
	 * diveded by 1000 -> 250 increments per ms
	 * hence, timer compare register to 250-1 -> ISR inc ms counter -> 1kHz loop
	 *
	 * configure timer 2:
	 */
	TCCR2A = (1<<WGM21);             // CTC mode
	TCCR2B = (1<<CS22);              // set prescaler to 64
	OCR2A = 249;                     // set timer compare register to 250-1
	TIMSK2 = (1<<OCIE2A);            // enable compare interrupt

	communication_ctrl<core_t, exts_t> com(core, exts);

	bool previous_state = false;
	core.init_sensors();
	while(1) /* main loop */
	{
		if (com.step()) {
			core.halt();
			start_bootloader();
		}
		if (current_state != previous_state) {
			led::red::set();   // red led on, begin of cycle
			core.step();
			supreme::adc::restart();
			led::red::reset(); // red led off, end of cycle
			previous_state = current_state;
		} else
			exts.step();
	}
	return 0;
}
