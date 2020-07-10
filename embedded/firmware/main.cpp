/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | October 2018                    |
 +---------------------------------*/

#include <avr/io.h>
#include <modm/platform.hpp>
#include <boards/sensorimotor_rev1_1.hpp>
#include <system/core.hpp>
#include <system/communication.hpp>
#include <system/adc.hpp>
#include <external/i2c_sensor.hpp>

using namespace std::chrono_literals;

/* FUSES ************************************************************** *
*                                                                       *
*        SUT0-----+ +-----CKSEL3                                        *
*        SUT1----+| |+----CKSEL2                                        *
*       CKOUT---+|| ||+---CKSEL1                                        *
*      CKDIV8--+||| |||+--CKSEL0                                        *
*              |||| ||||                                                *
* lfuse: 0xCF: 1100.1111                                                *
*        CKSEL3..1       = 111 : >8 Mhz external oscillator             *
*        CKSEL0, SUT1..0 = 100 : Ceramic Resonator, slowly rising power *
*        CKOUT           = 0   : no clock output                        *
* hfuse: 0xD9 (defaults)                                                *
* efuse: 0xFD: xxxx.x101 Brownout detection min:2V5 typ:2V7 max:2V9     *
* fuses explained: http://www.ladyada.net/learn/avr/fuses.html          *
* calculator: http://www.engbedded.com/fusecalc/                        *
*                                                                       *
* atmega328p defaults: lfuse: 0x62, hfuse: 0xD9, efuse 0x07 (0xFF)      *
* ********************************************************************* */
FUSES = {
	.low = 0xCF,
	.high = 0xD9,
	.extended = 0xFD,
};

modm::PeriodicTimer timer{1ms}; // render at 1kHz ideally

int main()
{
	Board::initialize();
	supreme::adc::init();
	supreme::adc::restart();

	typedef supreme::sensorimotor_core<supreme::motordriver_t> core_t;
	typedef supreme::ExternalSensor                            exts_t;
	core_t core;
	exts_t exts;

	unsigned long cycles = 0;

	supreme::communication_ctrl<core_t, exts_t> com(core, exts);

	core.init_sensors();
	while(1) /* main loop */
	{
		com.step();
		if (timer.execute()) {
			led::red::set();   // red led on, begin of cycle
			core.step();
			supreme::adc::restart();
			++cycles;
			led::red::reset(); // red led off, end of cycle
		} else
			exts.step();
	}
	return 0;
}
