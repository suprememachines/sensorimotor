namespace supreme {
namespace local_tests {

class test_sensorimotor_core {
public:

	void step() { }

	void set_target_pwm(uint8_t /*pwm*/) { }

	void set_target_dir(bool /*dir*/) { }

	void toggle_enable() { }
	void enable()  { }
	void disable() { }

	void toggle_full_release() { }

	uint16_t get_position        () { return 0x1A1B; }
	uint16_t get_current         () { return 0x2A2B; }
	uint16_t get_voltage_back_emf() { return 0x3A3B; }
	uint16_t get_voltage_supply  () { return 0x4A4B; }
	uint16_t get_temperature     () { return 0x5A5B; }
};

}} /* namespace supreme::local_tests */
