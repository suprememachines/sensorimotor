/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Rev 1.1 Firmware   |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 2021                    |
 +---------------------------------*/

#ifndef SUPREME_COMMUNICATION_HPP
#define SUPREME_COMMUNICATION_HPP

#include <xpcc/architecture/platform.hpp>
#include <system/eeprom.hpp>
#include <system/assert.hpp>
#include <system/sendbuffer.hpp>

namespace supreme {

using constants::assertion;

template <typename CoreType, typename ExternalSensorType>
class communication_ctrl {
public:
	enum command_id_t {
		no_command,
		data_requested,
		data_requested_response,
		set_voltage,
		ping,
		ping_response,
		set_id,
		set_id_response,
		set_pwm_limit,   /* no response */
		configure,
		configure_response,
		ext_sensor_request,
		ext_sensor_request_resp,
		raw_data,
		raw_data_response,
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
	CoreType&           ux;
	ExternalSensorType& exts;

	uint8_t             recv_buffer = 0;
	uint8_t             recv_checksum = 0;
	sendbuffer<16>      send;

	uint8_t             motor_id  = constants::max_id; // default
	uint8_t             target_id = constants::max_id;

	struct {
		bool default_dir = false;
	} config;

	/* motor related */
	bool                target_dir = false;
	uint8_t             target_pwm = 0;
	uint8_t             target_pwm_max = 0;

	struct {
		command_id_t     id    = no_command;
		command_state_t  state = syncing;
		unsigned int     bytes_received = 0;
	} cmd;

	bool                sync_state = false;
	uint8_t             num_bytes_read = 0;
	uint16_t            errors = 0;

	int                 exp_msg_length = -1;
	uint8_t             dat[256];

	/* reset com module if no new bytes have arrived
	   for several steps while in uart parsing. */
	uint8_t             watchdog = 0;

	/* signal from host to halt and jump to bootloader */
	bool                halt = false;

public:

	communication_ctrl(CoreType& ux, ExternalSensorType& exts)
	: ux(ux)
	, exts(exts)
	, send()
	{
		EEPROM::read_id(motor_id);
		config.default_dir = EEPROM::read_dir();
		ux.set_default_dir(config.default_dir);

		rs485::drive_enable::setOutput();
		rs485::drive_enable::reset();

		rs485::read_disable::setOutput();
		rs485::read_disable::reset();
	}

	inline
	bool byte_received(void) {
		bool result = Uart0::read(recv_buffer);
		if (result)
			recv_checksum += recv_buffer;
		return result;
	}

	command_state_t get_state()    const { return cmd.state; }
	uint8_t         get_motor_id() const { return motor_id; }
	uint16_t        get_errors()   const { return errors; }

	command_state_t waiting_for_id()
	{
		if (recv_buffer > constants::max_id) return error;
		switch(cmd.id)
		{
			/* single byte commands */
			case data_requested:
			case ping:
				return (motor_id == recv_buffer) ? verifying : eating;

			/* multi-byte commands */
			case set_voltage:
			case raw_data:
			case set_id:
			case set_pwm_limit:
			case configure:
			case ext_sensor_request:
				return (motor_id == recv_buffer) ? reading : eating;

			/* responses */
			case ping_response:
			case set_id_response:
			case configure_response:
			case data_requested_response:
			case ext_sensor_request_resp:
			case raw_data_response:
				return eating;

			default: /* unknown command */ break;
		}
		stop(assertion::waiting_for_id);
		return finished;
	}

	void prepare_data_response(void)
	{
		send.add_byte(0x80); /* 1000.0000 */
		send.add_byte(motor_id);
		send.add_word(ux.get_position());
		send.add_word(ux.get_current());
		send.add_word(ux.get_velocity());
		send.add_word(ux.get_voltage_supply());
		send.add_word(ux.get_temperature());
		//TODO: integrate voltage_back_emf?
		//TODO: integrate state/context fields
		//TODO: integrate error/status codes
	}

	command_state_t process_command()
	{
		switch(cmd.id)
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

			case ping:
				send.add_byte(0xE1); /* 1110.0001 */
				send.add_byte(motor_id);
				break;

			case raw_data:
				if (1 == exp_msg_length and 'S' == dat[0])
				{
					send.add_byte(0x56); /* 1101.0110 */
					send.add_byte(motor_id);
					send.add_byte(0x1); // one byte msg len
					send.add_byte('Y'); // send ack
					halt = true;
				}
				break;

			case set_id:
				EEPROM::update_id(target_id);
				motor_id = target_id;
				send.add_byte(0x71); /* 0111.0001 */
				send.add_byte(motor_id);
				break;

			case set_pwm_limit:
				ux.set_pwm_limit(target_pwm_max);
				/* no response needed */
				break;

			case configure:
				EEPROM::update_dir(config.default_dir);
				ux.set_default_dir(config.default_dir);
				send.add_byte(0x51); /* 0101.0001 */
				send.add_byte(motor_id);
				break;

			case ext_sensor_request:
				send.add_byte(0x41); /* 0100.0001 */
				send.add_byte(motor_id);
				{
					auto const& s = exts.get_values();
					send.add_word(s.x);
					send.add_word(s.y);
					send.add_word(s.z);
				}
				exts.restart();
				break;

			default: /* unknown command */
				stop(assertion::process_command);
				break;

		} /* switch cmd.id */

		return finished;
	}


	/* handle multi-byte commands */
	command_state_t waiting_for_data()
	{
		switch(cmd.id)
		{
			case set_voltage:
				target_pwm = recv_buffer;
				return verifying;

			case set_id:
				if (recv_buffer < 128) {
					target_id = recv_buffer;
					return verifying;
				}
				else
					return error;

			case set_pwm_limit:
				target_pwm_max = recv_buffer;
				return verifying;

			case raw_data:
				// first byte of raw data is the data length
				if (-1 == exp_msg_length)
					exp_msg_length = recv_buffer;
				else
					dat[num_bytes_read++] = recv_buffer;
				return (num_bytes_read < exp_msg_length) ? reading : verifying;

			case configure:
				config.default_dir = static_cast<bool>(recv_buffer & 0x1);
				return verifying;

			case ext_sensor_request:
				//ext_sensor_id = recv_buffer; TODO handle sensor id
				return verifying;

			default: /* unknown command */ break;
		}
		stop(assertion::waiting_for_data);
		return finished;
	}

	command_state_t eating_others_data()
	{
		++num_bytes_read; /* eat bytes, not reading actually */

		/*TODO: 'exp_msg_length' could be used here, too
		define for each command the num expected bytes and
		this functions will greatly simplify. */
		switch(cmd.id)
		{
			case data_requested:
			case ping:
			case ping_response:
			case set_id_response:
			case configure_response:
				assert(num_bytes_read == 1, assertion::single_byte_commands);
				return finished;

			case set_voltage:
			case set_id:
			case set_pwm_limit:
			case configure:
			case ext_sensor_request:
				return (num_bytes_read <  2) ? eating : finished;

			case ext_sensor_request_resp:
				return (num_bytes_read <  7) ? eating : finished;

			case data_requested_response:
				return (num_bytes_read < 11) ? eating : finished;

			case raw_data:
			case raw_data_response:
				if (-1 == exp_msg_length)
					exp_msg_length = recv_buffer;
				return (num_bytes_read < exp_msg_length+1) ? eating : finished;

			default: /* unknown command */ break;
		}
		stop(assertion::eating_others_data);
		return finished;
	}

	command_state_t verify_checksum()
	{
		return (recv_checksum == 0) ? pending : error;
	}

	command_state_t get_sync_bytes()
	{
		if (recv_buffer != constants::syncbyte) {
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
			case 0xC0: /* 1100.0000 */ cmd.id = data_requested;          break;

			/* multi-byte commands */
			case 0xB0: /* 1011.0000 */ //fall through
			case 0xB1: /* 1011.0001 */ cmd.id = set_voltage;
			                           target_dir = recv_buffer & 0x1;   break;
			case 0xE0: /* 1110.0000 */ cmd.id = ping;                    break;
			case 0xA0: /* 1010.0000 */ cmd.id = set_pwm_limit;           break;
			case 0x70: /* 0111.0000 */ cmd.id = set_id;                  break;
			case 0x50: /* 0101.0000 */ cmd.id = configure;               break;
			case 0x40: /* 0100.0000 */ cmd.id = ext_sensor_request;      break;

			/* read but ignore sensorimotor responses */
			case 0xE1: /* 1110.0001 */ cmd.id = ping_response;           break;
			case 0x71: /* 0111.0001 */ cmd.id = set_id_response;         break;
			case 0x51: /* 0101.0001 */ cmd.id = configure_response;      break;
			case 0x80: /* 1000.0000 */ cmd.id = data_requested_response; break;
			case 0x41: /* 0400.0001 */ cmd.id = ext_sensor_request_resp; break;

			case 0x55: /* 0101.0101 */ cmd.id = raw_data;                break;
			case 0x56: /* 0101.0110 */ cmd.id = raw_data_response;       break;

			default: /* unknown command */
				return finished;

		} /* switch recv_buffer */

		return get_id;
	}

	/* return code true means continue processing, false: wait for next byte */
	bool receive_command()
	{
		/* withdraw current attempt to parse bytes
		   if there is no new data for 100ms */
		if (syncing != cmd.state) watchdog++;
		if (watchdog > 100/*ms*/) cmd.state = error;

		switch(cmd.state)
		{
			case syncing:
				if (not byte_received()) return false;
				cmd.state = get_sync_bytes();
				break;

			case awaiting:
				if (not byte_received()) return false;
				cmd.state = search_for_command();
				break;

			case get_id:
				if (not byte_received()) return false;
				cmd.state = waiting_for_id();
				break;

			case reading:
				if (not byte_received()) return false;
				cmd.state = waiting_for_data();
				break;

			case eating:
				if (not byte_received()) return false;
				cmd.state = eating_others_data();
				break;

			case verifying:
				if (not byte_received()) return false;
				cmd.state = verify_checksum();
				break;

			case pending:
				cmd.state = process_command();
				break;

			case finished: /* cleanup, prepare for next message */
				send.flush();
				cmd.id = no_command;
				cmd.state = syncing;
				num_bytes_read = 0;
				recv_checksum = 0;
				exp_msg_length = -1;
				watchdog = 0;
				assert(sync_state == false, assertion::no_sync_in_finished);
				/* anything else todo? */
				break;

			case error:
				if (errors < 0xffff) ++errors;
				send.discard();
				cmd.state = finished;
				break;

			default: /* unknown command state */
				stop(assertion::unknown_command_state);
				break;

		} /* switch cmd.state */
		return true; // continue
	}

	inline
	bool step() {
		while(receive_command());
		return halt;
	}
};

} /* namespace supreme */

#endif /* SUPREME_COMMUNICATION_HPP */
