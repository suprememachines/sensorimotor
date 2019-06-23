/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_ADC_HPP
#define SUPREME_ADC_HPP

#include <avr/io.h>
#include <avr/interrupt.h>
#include <xpcc/architecture/platform.hpp>

/*
	+-------+-------+------------------------------------------+
	| REFS1 | REFS0 | Voltage Reference Selection              |
	+-------+-------+------------------------------------------+
	|     0 |     0 | AREF, internal Vref is turned off        |
	|     0 |     1 | AVCC with external capacitor at AREF pin |
	|     1 |     0 | reserved                                 |
	|     1 |     1 | int. 1V1 ref. with ext. cap at AREF pin  |
	+-------+-------+------------------------------------------+

	ADMUX Register
	+------+-------+-------+-------+-------+-------+-------+-------+-------+
	| bit  |     7 |     6 |     5 |     4 |     3 |     2 |     1 |     0 |
	| name | REFS1 | REFS0 | ADLAR |       |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
	+------+-------+-------+-------+-------+-------+-------+-------+-------+
*/

namespace supreme {
namespace adc {
	const uint8_t vref = (1 << REFS0); // select AVCC as reference

	const uint8_t position         = Board::adc_channel::position;
	const uint8_t current          = Board::adc_channel::current;
	const uint8_t voltage_back_emf = Board::adc_channel::voltage_back_emf;
	const uint8_t voltage_supply   = Board::adc_channel::voltage_supply;
	const uint8_t temperature      = Board::adc_channel::temperature;

	const uint8_t first = position;

	uint8_t next[8];

	/* registers changed by isr */
	volatile uint16_t result[8];
	volatile uint8_t  channel = first;
	volatile bool     conversion_finished = true;

	inline void set_channel(uint8_t ch){ ADMUX = adc::vref | ch; }
	inline void enable(void)           { ADCSRA |= 1<<ADEN; }
	inline void interrupt_enable(void) { ADCSRA |= 1<<ADIE; }
	inline void start_conversion(void) { ADCSRA |= 1<<ADSC; }

	inline void restart(void) {
		while(!conversion_finished);
		start_conversion();
		conversion_finished = false;
	}

	inline void set_clock(void) {
 		/*
			board clock is 16MHz, set prescaler to 128
			16.000kHz / 128 = 125kHz ADC clock
			(allowed range is 50..200kHz)
		*/
		ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	}

	inline void init() {

		//TODO init this at compile time
		next[position        ] = current;
		next[current         ] = voltage_back_emf;
		next[voltage_back_emf] = voltage_supply;
		next[voltage_supply  ] = temperature;
		next[temperature     ] = position;

		for (uint8_t i = 0; i < 8; ++i)
			result[i] = 0;

		set_channel(channel);
		set_clock();
		enable();
		interrupt_enable();
	}
}

ISR(ADC_vect)
{
	adc::result[adc::channel] = ADC;         // read result (10 bit)
	adc::channel = adc::next[adc::channel];  // select next channel
	adc::set_channel(adc::channel);          // multiplex adc

	if (adc::channel != adc::first)          // restart conversion
		adc::start_conversion();
	else
		adc::conversion_finished = true;
}

} /* namespace supreme */

#endif /* SUPREME_ADC_HPP */
