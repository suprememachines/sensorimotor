/*---------------------+
| Supreme Sensorimotor |
+---------------------*/


#ifndef XPCC_SUPREME_SENSORIMOTOR_HPP
#define XPCC_SUPREME_SENSORIMOTOR_HPP

#include <xpcc/architecture/platform.hpp>
#include <xpcc/debug/logger.hpp>

using namespace xpcc::atmega;

namespace Board
{

using systemClock = xpcc::avr::SystemClock;

// arduino compatible footprint
using A0 = GpioC0;
using A1 = GpioC1;
using A2 = GpioC2;
using A3 = GpioC3;
using A4 = GpioC4;
using A5 = GpioC5;

using D0  = GpioD0;
using D1  = GpioD1;
using D2  = GpioD2;
using D3  = GpioD3;
using D4  = GpioD4;
using D5  = GpioD5;
using D6  = GpioD6;
using D7  = GpioD7;
using D8  = GpioB0;
using D9  = GpioB1;
using D10 = GpioB2;
using D11 = GpioB3;
using D12 = GpioB4;
using D13 = GpioB5;

/* leds */
namespace led {
	using yellow = D5;
	using red    = D7;
}

/* communication */
namespace rs485 {
	using drive_enable = D3;
	using read_disable = A0; 
}

/* motor control outputs */
namespace motor {
	using VSO = D4; // enable power supply of h-bridge logic
	using DIR = D2; // set rotation direction
	using PWM = D9; // set pulse width modulation, motor strength
	using DIS = D6; // disable h-bridge bride
}

/* serial peripheral interface */
namespace spi {
	using CSN  = D10;
	using MOSI = D11;
	using MISO = D12;
	using SCK  = D13;
}


inline void
initialize()
{
	systemClock::enable();

	/* setup motor h-bridge */
	motor::VSO::setOutput();
	motor::DIR::setOutput();
	motor::PWM::setOutput(); //TODO how to enable PWM pin?
	motor::DIS::setOutput();

	/**TODO:
		+ how to enable PWM pins
		+ how to enable analog inputs
		+ how to enable UART send and recv
		+ how to enable VBUS usb
		+ control the motor via SPI
	*/

	D0::connect(Uart0::Rx);
	D1::connect(Uart0::Tx);
	Uart0::initialize<systemClock, 1000000>(); // 1Mbaud/s

	// xpcc::Clock initialization
	// Clear Timer on Compare Match Mode
	/*TCCR0A = (1 << WGM01);
	TIMSK0 = (1 << OCIE0A);
#if F_CPU   > 16000000
	// Set and enable output compare A
	OCR0A = F_CPU / (1000ul * 256);
	// Set prescaler 256 and enable timer
	TCCR0B = (1 << CS02);
#elif F_CPU > 2000000
	// Set and enable output compare A
	OCR0A = F_CPU / (1000ul * 64);
	// Set prescaler 64 and enable timer
	TCCR0B = (1 << CS01) | (1 << CS00);
#elif F_CPU > 1000000
	// Set and enable output compare A
	OCR0A = F_CPU / (1000ul * 8);
	// Set prescaler 64 and enable timer
	TCCR0B = (1 << CS00);
#endif
*/
	enableInterrupts();

}

}

using namespace Board;
extern xpcc::IOStream serialStream;

#endif	// XPCC_SUPREME_SENSORIMOTOR_HPP

