namespace supreme {
namespace local_tests {

struct ExternalSensor {
	struct Values {
		int16_t x = -1337;
		int16_t y = +2342;
		int16_t z = -4223;
	} values;

	unsigned ext_sensor_requests = 0;

	Values const& get_values(void) const { return values; }
	void restart() { ++ext_sensor_requests; }
};

class test_sensorimotor_core {
public:

	void step() { }

	void set_target_pwm(uint8_t pwm) { voltage_pwm = pwm; }
	void set_pwm_limit(uint8_t lim) { max_pwm = lim; }
	void set_target_dir(bool dir) { direction = dir; }

	void toggle_enable() { enabled = not enabled; }
	void enable()  { enabled = true; }
	void disable() { enabled = false; }

	uint16_t get_position        () { return 0x1A1B; }
	uint16_t get_current         () { return 0x2A2B; }
	uint16_t get_velocity        () { return 0x3A3B; }
	//uint16_t get_voltage_back_emf() { return 0x0;    } /* currently not in use */
	uint16_t get_voltage_supply  () { return 0x4A4B; }
	uint16_t get_temperature     () { return 0x5A5B; }

	uint8_t max_pwm = 0;
	uint8_t voltage_pwm = 0;
	bool    direction = false;
	bool    enabled = false;

	ExternalSensor sensor_ext;
};

}} /* namespace supreme::local_tests */
