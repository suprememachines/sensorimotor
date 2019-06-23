#include <system/communication.hpp>
#include <xpcc/architecture/platform.hpp>
#include "./catch_1.10.0.hpp"

#include <test_sensorimotor_core.hpp>

namespace supreme {
namespace local_tests {


template <typename T>
bool verify_checksum(T const& data) {
	uint8_t sum = 0;
	for (auto const& d : data)
		sum += d;
	return sum == 0;
}


TEST_CASE( "sendbuffer is filled and flushed", "[communication]")
{
	reset_hardware();
	sendbuffer<16> send;

	/* contains sync-bytes */
    REQUIRE( send.size() == 2 );
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	send.add_byte(0x87);
	REQUIRE( send.size() == 3 );

	send.add_word(0x1234);
	REQUIRE( send.size() == 5 );

	REQUIRE( not Uart0::buffer_flushed );
	rs485::stats.clear();

	send.flush();
	REQUIRE( Uart0::buffer_flushed );
	REQUIRE( rs485::stats.send_enable  == 1 );
	REQUIRE( rs485::stats.send_disable == 1 );
	REQUIRE( rs485::stats.recv_disable == 1 );
	REQUIRE( rs485::stats.recv_enable  == 1 );


	REQUIRE( Uart0::recv_buffer.size() == 5+1 ); // checksum was added automatically
	REQUIRE( send.size() == 2 ); // sendbuffer was reset

	/* check content of sent data */
	REQUIRE( Uart0::recv_buffer[0] == 0xff );
	REQUIRE( Uart0::recv_buffer[1] == 0xff );
	REQUIRE( Uart0::recv_buffer[2] == 0x87 );
	REQUIRE( Uart0::recv_buffer[3] == 0x12 );
	REQUIRE( Uart0::recv_buffer[4] == 0x34 );

	uint8_t chksum = 0;
	for (unsigned i = 0; i < Uart0::recv_buffer.size() - 1; ++i)
		chksum += Uart0::recv_buffer[i];

	chksum = ~chksum + 1;

	REQUIRE( chksum == Uart0::recv_buffer.back() );

	/* checksum adds to zero */
	uint8_t zero_sum = 0;
	for (auto b : Uart0::recv_buffer)
		zero_sum += b;

	REQUIRE( zero_sum == 0 );
}

TEST_CASE( "empty sendbuffer is not flushed", "[communication]")
{
	reset_hardware();
	sendbuffer<16> send;

	/* contains sync-bytes */
    REQUIRE( send.size() == 2 );
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	REQUIRE( not Uart0::buffer_flushed );
	rs485::stats.clear();

	send.flush();
	REQUIRE( not Uart0::buffer_flushed );
	REQUIRE( rs485::stats.send_enable  == 0 );
	REQUIRE( rs485::stats.send_disable == 0 );
	REQUIRE( rs485::stats.recv_disable == 0 );
	REQUIRE( rs485::stats.recv_enable  == 0 );

	REQUIRE( Uart0::recv_buffer.size() == 0 ); // nothing is send
}


TEST_CASE( "ping command can be received and is responded", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );

	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 1st sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 2nd sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::awaiting );
	Uart0::send_queue.push(0xe0); // ping cmd
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::get_id );
	Uart0::send_queue.push(  23); // motor id
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::verifying );
	Uart0::send_queue.push(0x0B); // checksum
	REQUIRE( not Uart0::buffer_flushed );
	com.step();

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	REQUIRE( Uart0::recv_buffer.size() == 5 );
	REQUIRE( Uart0::buffer_flushed );

	REQUIRE( Uart0::recv_buffer[0] == 0xff );
	REQUIRE( Uart0::recv_buffer[1] == 0xff );
	REQUIRE( Uart0::recv_buffer[2] == 0xe1 );
	REQUIRE( Uart0::recv_buffer[3] == 23   );
	REQUIRE( verify_checksum(Uart0::recv_buffer) );
}

TEST_CASE( "data request command can be received and is responded", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );

	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 1st sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 2nd sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::awaiting );
	Uart0::send_queue.push(0xC0); // data request
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::get_id );
	Uart0::send_queue.push(  23); // motor id
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::verifying );
	Uart0::send_queue.push(0x2B); // checksum
	REQUIRE( not Uart0::buffer_flushed );
	com.step();

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	REQUIRE( Uart0::recv_buffer.size() == 15 );
	REQUIRE( Uart0::buffer_flushed );

	REQUIRE( Uart0::recv_buffer[ 0] == 0xff );
	REQUIRE( Uart0::recv_buffer[ 1] == 0xff );
	REQUIRE( Uart0::recv_buffer[ 2] == 0x80 );
	REQUIRE( Uart0::recv_buffer[ 3] == 23   );
	REQUIRE( Uart0::recv_buffer[ 4] == 0x1A ); // position
	REQUIRE( Uart0::recv_buffer[ 5] == 0x1B );
	REQUIRE( Uart0::recv_buffer[ 6] == 0x2A ); // current
	REQUIRE( Uart0::recv_buffer[ 7] == 0x2B );
	REQUIRE( Uart0::recv_buffer[ 8] == 0x3A ); // voltage back emf
	REQUIRE( Uart0::recv_buffer[ 9] == 0x3B );
	REQUIRE( Uart0::recv_buffer[10] == 0x4A ); // voltage supply
	REQUIRE( Uart0::recv_buffer[11] == 0x4B );
	REQUIRE( Uart0::recv_buffer[12] == 0x5A ); // temperature
	REQUIRE( Uart0::recv_buffer[13] == 0x5B );

	REQUIRE( verify_checksum(Uart0::recv_buffer) );
}

void send(std::vector<uint8_t> buf) {
	Uart0::send_queue.push(0xff);
	Uart0::send_queue.push(0xff);
	uint8_t chksum = 0xfe;
	for (auto& b : buf) {
		chksum += b;
		Uart0::send_queue.push(b);
	}
	Uart0::send_queue.push(~chksum + 1);
}

TEST_CASE( "valid commands and responses for other motors is ignored", "[communication]")
{
	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	/* msg for different motor id (to be ignored) */
	std::vector<uint8_t> ping         = { 0xe0, 42 };
	std::vector<uint8_t> data_request = { 0xC0, 43 };
	std::vector<uint8_t> set_id       = { 0x70, 44, 13 };
	std::vector<uint8_t> set_pwm_limit= { 0xA0, 37, 255 };

	/* msg responses from different motor ids */
	std::vector<uint8_t> re_ping         = { 0xe1, 42 };
	std::vector<uint8_t> re_data_request = { 0x80, 43, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::vector<uint8_t> re_set_id       = { 0x71, 13 };

	reset_hardware();
	core_t ux;
	exts_t ex;
	com_t com(ux, ex);
	REQUIRE( com.get_motor_id() == 23 );

	for (auto const& cmd : { ping
	                       , data_request
	                       , set_id
	                       , set_pwm_limit
	                       , re_ping
	                       , re_data_request
	                       , re_set_id
	                       } )
	{
		reset_hardware();
		com.step();
		REQUIRE( Uart0::recv_buffer.size() == 0 );
		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );

		send(cmd);

		REQUIRE( not Uart0::buffer_flushed );
		com.step();

		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );
		REQUIRE( com.get_errors() == 0 );
		REQUIRE( Uart0::recv_buffer.size() == 0 );
		REQUIRE( not Uart0::buffer_flushed );
	}
}

TEST_CASE( "set_id command can be received, id is set and command is responded", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );
	uint8_t new_id = 1;

	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 1st sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	Uart0::send_queue.push(0xff); // 2nd sync
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::awaiting );
	Uart0::send_queue.push(0x70); // set_id cmd
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::get_id );
	Uart0::send_queue.push(  23); // motor id
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::reading );
	Uart0::send_queue.push(new_id); // motor id
	com.step();

	REQUIRE( com.get_state() == com_t::command_state_t::verifying );
	Uart0::send_queue.push(0x7A); // checksum
	REQUIRE( not Uart0::buffer_flushed );
	com.step();

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	REQUIRE( Uart0::recv_buffer.size() == 5 );
	REQUIRE( Uart0::buffer_flushed );

	REQUIRE( com.get_motor_id() == new_id );

	REQUIRE( Uart0::recv_buffer[0] == 0xff );
	REQUIRE( Uart0::recv_buffer[1] == 0xff );
	REQUIRE( Uart0::recv_buffer[2] == 0x71 );
	REQUIRE( Uart0::recv_buffer[3] == new_id);
	REQUIRE( verify_checksum(Uart0::recv_buffer) );
}

TEST_CASE( "invalid set_id command is refused when checksum is wrong", "[communication]")
{
	reset_hardware();
	set_motor_id(1);

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 1 );
	uint8_t new_id = 42;

	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	Uart0::send_queue.push(0xff); // 1st sync
	Uart0::send_queue.push(0xff); // 2nd sync
	Uart0::send_queue.push(0x70); // set_id cmd
	Uart0::send_queue.push(   1); // motor id
	Uart0::send_queue.push(new_id); // motor id
	Uart0::send_queue.push(0xff); // invalid checksum

	com.step();

	REQUIRE( com.get_errors() == 1 );
	REQUIRE( Uart0::recv_buffer.size() == 0 );

	REQUIRE( com.get_motor_id() != new_id );
	REQUIRE( com.get_motor_id() == 1 );

}

TEST_CASE( "invalid set_id command is refused when new id is wrong", "[communication]")
{
	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;

	std::vector<uint8_t> wrong0 = { 0x70, 1, 128  };
	std::vector<uint8_t> wrong1 = { 0x70, 1, 255  };

	std::vector<uint8_t> correct0 = { 0x70, 1, 13  };
	std::vector<uint8_t> correct1 = { 0x70, 1, 127 };
	std::vector<uint8_t> correct2 = { 0x70, 1, 0   };

	for (auto const& cmd : { correct0, correct1, correct2} ) {
		reset_hardware();
		set_motor_id(1);
		com_t com(ux, ex);
		REQUIRE( com.get_motor_id() == 1 );

		send(cmd);
		com.step();
		REQUIRE( com.get_errors() == 0 );
		REQUIRE( com.get_motor_id() == cmd[2] );
	}

	for (auto const& cmd : { wrong0, wrong1} ) {
		reset_hardware();
		set_motor_id(1);
		com_t com(ux, ex);
		REQUIRE( com.get_motor_id() == 1 );

		send(cmd);
		com.step();
		REQUIRE( com.get_errors() == 1 );
		REQUIRE( com.get_motor_id() != cmd[2] );
		REQUIRE( com.get_motor_id() == 1 );
	}
}

TEST_CASE( "multiple pings", "[communication]")
{
	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	/* msg for different motor id (to be ignored) */
	std::vector<uint8_t> ping     = { 0xe0, 42 };

	/* msg responses from different motor ids */
	std::vector<uint8_t> re_ping  = { 0xe1, 42 };

	reset_hardware();
	set_motor_id(23);
	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );

	for (unsigned id = 0; id < 128; ++id)
	{
		Uart0::recv_buffer.clear();
		Uart0::buffer_flushed = false;

		REQUIRE( Uart0::recv_buffer.size() == 0 );
		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );

		ping[1] = id;
		send(ping);
		if (id != 23) {
			re_ping[1] = id;
			send(re_ping);
		}

		REQUIRE( not Uart0::buffer_flushed );
		com.step();

		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );
		REQUIRE( com.get_errors() == 0 );

		if (id != 23) {
			REQUIRE( Uart0::recv_buffer.size() == 0 );
			REQUIRE( not Uart0::buffer_flushed );
		} else {
			REQUIRE( Uart0::recv_buffer.size() == 5 );
			REQUIRE( Uart0::buffer_flushed );
		}
	}
}


TEST_CASE( "multiple data requests", "[communication]")
{
	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	/* msg for different motor id (to be ignored) */
	std::vector<uint8_t> data_request     = { 0xC0, 42 };

	/* msg responses from different motor ids */
	std::vector<uint8_t> re_data_request  = { 0x80, 43, 0, 1, 2, 0xff, 0xff, 0xC0, 6, 7, 8, 9 };

	reset_hardware();
	set_motor_id(23);
	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );

	for (unsigned id = 0; id < 128; ++id)
	{
		Uart0::recv_buffer.clear();
		Uart0::buffer_flushed = false;

		REQUIRE( Uart0::recv_buffer.size() == 0 );
		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );

		data_request[1] = id;
		send(data_request);
		if (id != 23) {
			re_data_request[1] = id;
			send(re_data_request);
		}

		REQUIRE( not Uart0::buffer_flushed );
		com.step();

		REQUIRE( Uart0::send_queue.empty() );
		REQUIRE( com.get_state() == com_t::command_state_t::syncing );
		REQUIRE( com.get_errors() == 0 );

		if (id != 23) {
			REQUIRE( Uart0::recv_buffer.size() == 0 );
			REQUIRE( not Uart0::buffer_flushed );
		} else {
			REQUIRE( Uart0::recv_buffer.size() == 15 );
			REQUIRE( Uart0::buffer_flushed );
		}
	}
}

TEST_CASE( "find valid messages in garbage", "[communication]")
{
	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	std::vector<uint8_t> ping         = { 0xe0, 23 };
	std::vector<uint8_t> data_request = { 0xC0, 23 };
	std::vector<uint8_t> set_id       = { 0x70, 23, 23 };
	std::vector<uint8_t> set_pwm_limit= { 0xA0, 38, 196 };

	/* msg responses from different motor ids */
	std::vector<uint8_t> re_ping         = { 0xe1, 42 };
	std::vector<uint8_t> re_data_request = { 0x80, 43, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::vector<uint8_t> re_set_id       = { 0x71, 13 };

	std::vector<uint8_t> garbage      = { 0xff, 0xdd, 0xff, 0x34, 0xe1, 23, 0xff, 0xfe, 0x03 };

	reset_hardware();
	set_motor_id(23);
	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );

	for (auto const& cmd : { ping
	                       , data_request
	                       , set_id
	                       , set_pwm_limit
	                       , re_ping
	                       , re_data_request
	                       , re_set_id
	                       } )
	{
		for (auto& g: garbage) Uart0::send_queue.push(g);
		send(cmd);
	}

	Uart0::recv_buffer.clear();
	Uart0::buffer_flushed = false;

	REQUIRE( com.get_state() == com_t::command_state_t::syncing );

	REQUIRE( not Uart0::buffer_flushed );
	com.step();

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	REQUIRE( Uart0::recv_buffer.size() == 5 + 15 + 5/* TODO: detect cmd response */ );
	REQUIRE( Uart0::buffer_flushed );
}

TEST_CASE( "set_pwm_limit command can be received, pwm limit is set and command is NOT responded", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );
	uint8_t new_id = 1;

	std::vector<uint8_t> set_pwm_limit_cmd = { 0xA0, 23, 196 };

	reset_hardware();
	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );
	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );

	send(set_pwm_limit_cmd);

	REQUIRE( not Uart0::buffer_flushed );

	REQUIRE( ux.max_pwm == 0 );

	com.step();

	REQUIRE( ux.max_pwm == 196 );

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );
	REQUIRE( Uart0::recv_buffer.size() == 0 );
	REQUIRE( not Uart0::buffer_flushed );

	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );
}

TEST_CASE( "set_voltage command can be received, pwm and direction is set and command is responded with data", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );
	uint8_t new_id = 1;

	std::vector<uint8_t> set_voltage_cmd = { 0xB1, 23, 64 };

	reset_hardware();
	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );
	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );

	send(set_voltage_cmd);

	REQUIRE( not Uart0::buffer_flushed );

	REQUIRE( ux.voltage_pwm == 0 );
	REQUIRE( ux.direction == false );

	com.step();

	REQUIRE( ux.voltage_pwm == 64 );
	REQUIRE( ux.direction == true );

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	/* received data package */
	REQUIRE( Uart0::recv_buffer.size() == 15 );
	REQUIRE( Uart0::recv_buffer[2] == 0x80 );
	REQUIRE( Uart0::recv_buffer[3] == 23 );
	REQUIRE( Uart0::buffer_flushed );
}

int16_t get_signed_word(uint8_t hi, uint8_t lo) { return (hi << 8) | lo; }

TEST_CASE( "ext_sensor_request command can be received and is responded with data", "[communication]")
{
	reset_hardware();

	using core_t = test_sensorimotor_core;
	using exts_t = ExternalSensor;
	using com_t = supreme::communication_ctrl<core_t, exts_t>;

	core_t ux;
	exts_t ex;
	com_t com(ux, ex);

	REQUIRE( com.get_motor_id() == 23 );
	uint8_t new_id = 1;

	std::vector<uint8_t> ext_sensor_req_cmd = { 0x40, /*motor_id=*/23, /*sensor_id=*/01 };

	reset_hardware();
	com.step();
	REQUIRE( Uart0::recv_buffer.size() == 0 );
	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );

	send(ext_sensor_req_cmd);

	REQUIRE( not Uart0::buffer_flushed );

	REQUIRE( ex.ext_sensor_requests == 0 );

	com.step();

	REQUIRE( ex.ext_sensor_requests == 1 );

	REQUIRE( Uart0::send_queue.empty() );
	REQUIRE( com.get_state() == com_t::command_state_t::syncing );
	REQUIRE( com.get_errors() == 0 );

	/* received sensor data package */
	REQUIRE( Uart0::recv_buffer.size() == 11 );
	REQUIRE( Uart0::recv_buffer[2] == 0x41 );
	REQUIRE( Uart0::recv_buffer[3] == 23 );

	REQUIRE( get_signed_word( Uart0::recv_buffer[4]
	                        , Uart0::recv_buffer[5] ) == -1337 );
	REQUIRE( get_signed_word( Uart0::recv_buffer[6]
	                        , Uart0::recv_buffer[7] ) == +2342 );
	REQUIRE( get_signed_word( Uart0::recv_buffer[8]
	                        , Uart0::recv_buffer[9] ) == -4223 );
	REQUIRE( Uart0::buffer_flushed );
}

}} /* namespace supreme::local_tests */
