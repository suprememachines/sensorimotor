/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Rev. 1.1 Firmware  |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 2021                    |
 +---------------------------------*/

#ifndef SUPREME_SENDBUFFER_HPP
#define SUPREME_SENDBUFFER_HPP

#include <xpcc/architecture/platform.hpp>
#include <system/assert.hpp>
#include <system/constants.hpp>

namespace supreme {

template <unsigned N>
class sendbuffer {

	static constexpr unsigned NumSyncBytes = 2;
	static constexpr uint8_t chk_init = (constants::syncbyte + constants::syncbyte) % 256;
	uint16_t  ptr = NumSyncBytes;
	uint8_t   buffer[N];
	uint8_t   checksum = chk_init;

public:
	sendbuffer()
	{
		static_assert(N > NumSyncBytes, "Invalid buffer size.");
		for (uint8_t i = 0; i < NumSyncBytes; ++i)
			buffer[i] = constants::syncbyte; // init sync bytes once
	}

	void add_byte(uint8_t byte) {
		assert(ptr < (N-1), constants::assertion::buffer_not_exceeded);
		buffer[ptr++] = byte;
		checksum += byte;
	}

	void add_word(uint16_t word) {
		add_byte((word  >> 8) & constants::syncbyte);
		add_byte( word        & constants::syncbyte);
	}

	void discard(void) { ptr = NumSyncBytes; }

	void flush() {
		if (ptr == NumSyncBytes) return;
		add_checksum();
		send_mode();
		Uart0::write(buffer, ptr);
		Uart0::flushWriteBuffer();    //TODO do not wait here, set an flag and check in com step to switch into recv mode again
		receive_mode();
		/* prepare next */
		ptr = NumSyncBytes;
	}

	uint16_t size(void) const { return ptr; }

private:

	void add_checksum() {
		assert(ptr < N, constants::assertion::buffer_not_exceeded);
		buffer[ptr++] = ~checksum + 1; /* two's complement checksum */
		checksum = chk_init;
	}

	void send_mode() {
		xpcc::delayNanoseconds(50); // wait for signal propagation
		rs485::read_disable::set();
		rs485::drive_enable::set();
		xpcc::delayMicroseconds(1); // wait at least one bit after enabling the driver
	}

	// TODO consider using loopback method and hence wait one byte before recv mode
	void receive_mode() {
		xpcc::delayMicroseconds(1); // wait at least one bit before disabling the driver
		rs485::read_disable::reset();
		rs485::drive_enable::reset();
		xpcc::delayNanoseconds(70); // wait for signal propagation
	}
};

} /* namespace supreme */

#endif /* SUPREME_SENDBUFFER_HPP */
