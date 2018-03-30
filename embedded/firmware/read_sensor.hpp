#ifndef SUPREME_READ_SENSOR_HPP
#define SUPREME_READ_SENSOR_HPP

/* read_sensor.hpp */
#include <xpcc/architecture/platform.hpp>


namespace supreme {

template <unsigned ChannelNumber>
class read_sensor {
    typedef xpcc::avr::SystemClock clock;
    uint16_t value = 0;

public:
    read_sensor() {
        /* initialize ADC */
        Adc::initialize<clock, 125000>();
        Adc::setReference(Adc::Reference::InternalVcc);
        value = Adc::readChannel(ChannelNumber);
    }

    void step() { value = Adc::readChannel(ChannelNumber); }

    uint16_t get_value() const { return value; }
};

} /* namespace supreme */

#endif /* SUPREME_READ_SENSOR_HPP */
