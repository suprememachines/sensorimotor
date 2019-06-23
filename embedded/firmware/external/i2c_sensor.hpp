/*---------------------------------+
 | Supreme Machines                |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | October 2018                    |
 +---------------------------------*/

#ifndef SUPREME_I2C_SENSOR_HPP
#define SUPREME_I2C_SENSOR_HPP

#include <xpcc/architecture/platform.hpp>
#include <xpcc/processing.hpp>
#include <xpcc/processing/resumable.hpp>
#include <xpcc/architecture/interface/i2c_master.hpp>
#include <external/adxl345.hpp>

namespace supreme {

/*----------------------------------------------------------------------+
 | ExternalSensor                                                       |
 | This is a proxy class for selecting and loading an external sensor.  |
 | Currently only 1 sensor supported, namely the ADXL345, but is to be  |
 | extended for other sensors.                                          |
 | TODO: Load sensor selection from EEPROM.                             |
 +----------------------------------------------------------------------*/

template <typename SensorType, typename DataType>
class ReaderThread : public xpcc::pt::Protothread, public xpcc::Resumable<1>
{
	SensorType& s;
	DataType& d;
	bool next_readout = false;

public:

	ReaderThread(SensorType& s, DataType& d) : s(s), d(d) {}

	bool update()
	{
		PT_BEGIN();
		while (true)
		{
			PT_WAIT_UNTIL( next_readout == true );
			PT_CALL( step() ); /* read out the sensor */
			next_readout = false;
		}
		PT_END();
	}

	void restart(void) { next_readout = true; }

private:

	xpcc::ResumableResult<void> step()
	{
		RF_BEGIN(0);

		s.readAccelerometer(); /* trigger new reading */
		while (not s.isNewDataAvailable())
			RF_CALL( s.update() );

		store_values();        /* backup values and.. */
		RF_END();
	}

	void store_values(void) {
		int16_t *vacc = reinterpret_cast<int16_t*>(s.getData());
		d.x = vacc[0];
		d.y = vacc[1];
		d.z = vacc[2];
	}
}; /* ReaderThread */


class ExternalSensor
{
	uint8_t data[16];

	typedef xpcc::Adxl345<I2cMaster> sensor_t;
	typedef struct Values { int16_t x,y,z; } data_t;

	sensor_t sensor;
	data_t values;

	ReaderThread<sensor_t, data_t> reader;

public:

	ExternalSensor()
	: sensor(data, /*address=*/0x53)
	, values()
	, reader(sensor, values)
	{
		sensor.configure();
	}

	void step(void) { reader.update(); }

	void restart(void) { reader.restart(); }

	Values const& get_values(void) const { return values; }

}; /* ExternalSensor */

} /* namespace supreme */

#endif /* SUPREME_I2C_SENSOR_HPP */
