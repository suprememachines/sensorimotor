/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Revision 1.1       |
 | Board Layout Configuration      |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | Last Modified: October 2018     |
 +---------------------------------*/


#ifndef SUPREME_SENSORIMOTOR_REV1_1_HPP
#define SUPREME_SENSORIMOTOR_REV1_1_HPP

#include <xpcc/architecture/platform.hpp>

using namespace xpcc::atmega;

namespace Board
{

using systemClock = xpcc::avr::SystemClock;

/* Arduino-compatible pin names */
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


namespace adc_channel {
	const uint8_t position         = 1,
	              current          = 7,
	              voltage_back_emf = 3,
	              voltage_supply   = 6,
	              temperature      = 2;
}

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

/* inter integrated circuit */
namespace i2c {
	using SDA = A4;
	using SCL = A5;
}

inline void
initialize()
{
	systemClock::enable();

	/* setup LEDs */
	led::yellow::setOutput();
	led::red::setOutput();

	/* setup motor h-bridge */
	motor::VSO::setOutput();
	motor::DIR::setOutput();
	motor::PWM::setOutput();
	motor::DIS::setOutput();

	/* connect and setup uart */
	D0::setInput(Gpio::InputType::PullUp);
	D0::connect(Uart0::Rx);
	D1::connect(Uart0::Tx);
	Uart0::initialize<systemClock, Uart0::Baudrate::MBps1>(); // 1Mbaud/s

	/* connect and setup I2C */
	i2c::SDA::connect(I2cMaster::Sda);
	i2c::SCL::connect(I2cMaster::Scl);
	I2cMaster::initialize<systemClock, I2cMaster::Baudrate::Fast>();

	enableInterrupts();
}

} /* namespace Board */

using namespace Board;

#include <motor/ifx9201sg.hpp>


#endif /* SUPREME_SENSORIMOTOR_REV1_1_HPP */
