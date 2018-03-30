#ifndef SUPREME_COMMUNICATION
#define SUPREME_COMMUNICATION

#include <xpcc/architecture/platform.hpp>
#include <avr/eeprom.h>
#include <assert.hpp>

#include <communication/sendbuffer.hpp>

/*
TODO: create new scheme for command processing:

	0) get sync bytes
	1) detect command
	2) look up expected number of bytes
	3) read all bytes (including checksum)
	4) verify checksum
	5) process command (or discard)
		+ dicard if
			- ID does not match
			- checksum is incorrect
			- timeout in byte stream

	consider having a class for each command, derived from a (virtual) base class

	TODO: clear recv buffer after timeout
*/
namespace supreme {

template <typename CoreType>
class communication_ctrl {
public:
	enum command_id_t { //TODO: this should be classes
		no_command,
		data_requested,
		data_requested_response,
		set_voltage,
		toggle_led,
		ping,
		ping_response,
		set_id,
		set_id_response,
	};

	enum command_state_t {
		syncing   = 0,
		awaiting  = 1,
		get_id    = 2,
		reading   = 3,
		eating    = 4,
		verifying = 5,
		pending   = 6,
		finished  = 7,
		error     = 8,
	};

private:
	CoreType&                    ux;
	uint8_t                      recv_buffer = 0;
	uint8_t                      recv_checksum = 0;
	sendbuffer<16>               send;

	uint8_t                      motor_id = 127; // set to default
	uint8_t                      target_id = 127;

	/* motor related */
	bool                         target_dir = false;
	uint8_t                      target_pwm = 0;

	/* TODO struct? */
	command_id_t                 cmd_id    = no_command;
	command_state_t              cmd_state = syncing;
	unsigned int                 cmd_bytes_received = 0;

	bool                         led_state = false;
	bool                         sync_state = false;

	uint8_t                      num_bytes_eaten = 0;
	uint16_t                     errors = 0;

public:

	communication_ctrl(CoreType& ux)
	: ux(ux)
	, send()
	{
		read_id_from_EEPROM();

		rs485::drive_enable::setOutput();
		rs485::drive_enable::reset();

		rs485::read_disable::setOutput();
		rs485::read_disable::reset();
	}

	// TODO move to eeprom/memory class
	void read_id_from_EEPROM() {
		eeprom_busy_wait();
		uint8_t read_id = eeprom_read_byte((uint8_t*)23);
		if (read_id) /* MSB is set, check if this id was written before */
			motor_id = read_id & 0x7F;
	}

	void write_id_to_EEPROM(uint8_t new_id) {
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t*)23, (new_id | 0x80));
	}

	bool byte_received(void) {
		bool result = Uart0::read(recv_buffer);
		if (result)
			recv_checksum += recv_buffer;
		return result;
	}

	command_state_t get_state()    const { return cmd_state; }
	uint8_t         get_motor_id() const { return motor_id; }
	uint16_t        get_errors()   const { return errors; }

	command_state_t waiting_for_id()
	{
		if (recv_buffer > 127) return error;
		switch(cmd_id)
		{
			case data_requested:
			case toggle_led:
			case ping:
				return (motor_id == recv_buffer) ? verifying : eating;

			case set_voltage:
			case set_id:
				return (motor_id == recv_buffer) ? reading : eating;

			/* responses */
			case ping_response:           return eating;
			case set_id_response:         return eating;
			case data_requested_response: return eating;

			default: /* unknown command */ break;
		}
		assert(false, 3);
		return finished;
	}

	void prepare_data_response(void)
	{
		send.add_byte(0x80); /* 1000.0000 */
		send.add_byte(motor_id);
		send.add_word(ux.get_position());
		send.add_word(ux.get_current());
		send.add_word(ux.get_voltage_back_emf());
		send.add_word(ux.get_voltage_supply());
		send.add_word(ux.get_temperature());
		//TODO: external I2C sensor
		//TODO: integrate enable_status
		//TODO: integrate error/status codes
	}

	command_state_t process_command()
	{
		switch(cmd_id)
		{
			case data_requested:
				ux.disable();
				ux.set_target_pwm(0);
				prepare_data_response();
				break;

			case set_voltage:
				ux.set_target_pwm(target_pwm);
				ux.set_target_dir(target_dir);
				ux.enable();
				prepare_data_response();
				break;

			case toggle_led: //TODO: apply pwm to LED
				if (led_state) {
					led::yellow::reset();
					led_state = false;
				}
				else {
					led::yellow::set();
					led_state = true;
				}
				break;

			case ping:
				send.add_byte(0xE1); /* 1110.0001 */
				send.add_byte(motor_id);
				break;

			case set_id:
				write_id_to_EEPROM(target_id);
				read_id_from_EEPROM();
				send.add_byte(0x71); /* 0111.0001 */
				send.add_byte(motor_id);
				break;

			default: /* unknown command */
				assert(false, 2);
				break;

		} /* switch cmd_id */

		return finished;
	}


	/* handle multi-byte commands */
	command_state_t waiting_for_data()
	{
		switch(cmd_id)
		{
			case set_voltage:
				target_pwm = recv_buffer;
				return verifying;

			case set_id:
				if (recv_buffer < 128) {
					target_id = recv_buffer;
					return verifying;
				}
				else return error;

			default: /* unknown command */ break;
		}
		assert(false, 4);
		return finished;
	}

	command_state_t eating_others_data()
	{
		++num_bytes_eaten;
		switch(cmd_id)
		{
			case data_requested:
			case toggle_led:
			case ping:
			case ping_response:
			case set_id_response:
				assert(num_bytes_eaten == 1, 77);
				return finished;

			case set_voltage:
			case set_id:
				return (num_bytes_eaten <  2) ? eating : finished;

			case data_requested_response:
				return (num_bytes_eaten < 11) ? eating : finished;

			default: /* unknown command */ break;
		}
		assert(false, 5);
		return finished;
	}

	command_state_t verify_checksum()
	{
		return (recv_checksum == 0) ? pending : error;
	}

	command_state_t get_sync_bytes()
	{
		if (recv_buffer != 0xFF) {
			sync_state = false;
			return finished;
		}

		if (sync_state) {
			sync_state = false;
			return awaiting;
		}

		sync_state = true;
		return syncing;
	}

	command_state_t search_for_command()
	{
		switch(recv_buffer)
		{
			/* single byte commands */
			case 0xC0: /* 1100.0000 */ cmd_id = data_requested;  break;
			case 0xD0: /* 1101.0000 */ cmd_id = toggle_led;      break;

			/* multi-byte commands */
			case 0xB0: /* 1011.0000 */ //fall through
			case 0xB1: /* 1011.0001 */ cmd_id = set_voltage;
			                           target_dir = recv_buffer & 0x1; break;
			case 0xE0: /* 1110.0000 */ cmd_id = ping;            break;
			case 0x70: /* 0111.0000 */ cmd_id = set_id;          break;

			/* read but ignore sensorimotor responses */
			case 0xE1: /* 1110.0001 */ cmd_id = ping_response;   break;
			case 0x71: /* 0111.0001 */ cmd_id = set_id_response; break;
			case 0x80: /* 1000.0000 */ cmd_id = data_requested_response; break;

			default: /* unknown command */
				return error;

		} /* switch recv_buffer */

		return get_id;
	}

	/* return code true means continue processing, false: wait for next byte */
	bool receive_command()
	{
		switch(cmd_state)
		{
			case syncing:
				if (not byte_received()) return false;
				cmd_state = get_sync_bytes();
				break;

			case awaiting:
				if (not byte_received()) return false;
				cmd_state = search_for_command();
				break;

			case get_id:
				if (not byte_received()) return false;
				cmd_state = waiting_for_id();
				break;

			case reading:
				if (not byte_received()) return false;
				cmd_state = waiting_for_data();
				break;

			case eating:
				if (not byte_received()) return false;
				cmd_state = eating_others_data();
				break;

			case verifying:
				if (not byte_received()) return false;
				cmd_state = verify_checksum();
				break;

			case pending:
				cmd_state = process_command();
				break;

			case finished: /* cleanup, prepare for next message */
				send.flush();
				cmd_id = no_command;
				cmd_state = syncing;
				num_bytes_eaten = 0;
				recv_checksum = 0;
				assert(sync_state == false, 55);
				/* anything else todo? */
				break;

			case error:
				if (errors < 0xffff) ++errors;
				led::yellow::set();
				send.discard();
				cmd_state = finished;
				break;

			default: /* unknown command state */
				assert(false, 17);
				break;

		} /* switch cmd_state */
		return true; // continue
	}

	void step() {
		while(receive_command());
	}
};

} /* namespace supreme */

#endif /* SUPREME_COMMUNICATION */
