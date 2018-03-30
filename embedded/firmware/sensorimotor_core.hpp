#ifndef SUPREME_SENSORIMOTOR_CORE_HPP
#define SUPREME_SENSORIMOTOR_CORE_HPP

#include <xpcc/architecture/platform.hpp>

#include <constants.hpp>
#include <motor_ifx9201sg.hpp>
#include <adc.hpp>
#include <temperature.hpp>

namespace supreme {

struct Sensors {

	uint16_t position;
	uint16_t current;
	uint16_t voltage_back_emf;
	uint16_t voltage_supply;
	uint16_t temperature;

	void step(void) {
		position         = adc::result[adc::position        ];
		current          = adc::result[adc::current         ];
		voltage_back_emf = adc::result[adc::voltage_back_emf];
		voltage_supply   = adc::result[adc::voltage_supply  ];
		temperature      = adc::result[adc::temperature     ];
		temperature      = get_temperature_celsius(adc::result[adc::temperature]);
	}
};

class sensorimotor_core {

	bool enabled;

	struct {
		uint8_t pwm;
		bool    dir;
	} target;

	Sensors          sensors;
	motor_ifx9201sg  motor;

public:

	sensorimotor_core()
	: enabled(false)
	, target()
	, sensors()
	, motor()
	{
		motor.disable();
		motor.set_pwm(0);
	}

	void apply_target_values(void) {
		if (enabled) {
			motor.set_pwm(target.pwm);
			motor.set_dir(target.dir);
			motor.enable();
		} else {
			motor.set_pwm(0);
			motor.disable();
			target.pwm = 0;
		}
	}

	void step(void) {
		apply_target_values();
		sensors.step();
		//const uint16_t value = sensors.position.get_value();

		/* simple test controller, toggles between max bounds */
		/*if (dir_left && value < lower_bound) {
			dir_left = false;
			motor.toggle_direction();
		} else if (!dir_left && value > upper_bound){
			dir_left = true;
			motor.toggle_direction();
		}*/

		/** release_mode */
		/*if (release_mode) {
			if (last_position < value) {
				set_dir(false);
				set_pwm(20);
			} else if (last_position > value) {
				set_dir(true);
				set_pwm(20);
			} else
				set_pwm(0);
		}
		last_position = value;
		*/
	}

	void set_target_pwm(uint8_t pwm) { if (pwm <= 128  ) target.pwm = pwm; }
	void set_target_dir(bool    dir) { target.dir = dir; }

	void enable()  { enabled = true;  }
	void disable() { enabled = false; }
	bool is_enabled() const { return enabled; }

	uint16_t get_position        () { return sensors.position; }
	uint16_t get_current         () { return sensors.current; }
	uint16_t get_voltage_back_emf() { return sensors.voltage_back_emf; }
	uint16_t get_voltage_supply  () { return sensors.voltage_supply; }
	uint16_t get_temperature     () { return sensors.temperature; }
};

} /* namespace supreme */

#endif /* SUPREME_SENSORIMOTOR_CORE_HPP */
