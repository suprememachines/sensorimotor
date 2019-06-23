/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_SENDBUFFER_HPP
#define SUPREME_SENDBUFFER_HPP

#include <xpcc/architecture/platform.hpp>
#include <system/assert.hpp>

namespace supreme {

template <unsigned N>
class sendbuffer {
	static const unsigned NumSyncBytes = 2;
	static const uint8_t chk_init = 0xFE; /* (0xff + 0xff) % 256 */
	uint16_t  ptr = NumSyncBytes;
	uint8_t   buffer[N];
	uint8_t   checksum = chk_init;
public:
	sendbuffer()
	{
		static_assert(N > NumSyncBytes, "Invalid buffer size.");
		for (uint8_t i = 0; i < NumSyncBytes; ++i)
			buffer[i] = 0xFF; // init sync bytes once
	}
	void add_byte(uint8_t byte) {
		assert(ptr < (N-1), 1);
		buffer[ptr++] = byte;
		checksum += byte;
	}
	void add_word(uint16_t word) {
		add_byte((word  >> 8) & 0xff);
		add_byte( word        & 0xff);
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
		assert(ptr < N, 8);
		buffer[ptr++] = ~checksum + 1; /* two's complement checksum */
		checksum = chk_init;
	}

	// TODO move to (future) communication interface class
	void send_mode() {
		xpcc::delayNanoseconds(50); // wait for signal propagation
		rs485::read_disable::set();
		rs485::drive_enable::set();
		xpcc::delayMicroseconds(1); // wait at least one bit after enabling the driver
	}
	void receive_mode() {
		xpcc::delayMicroseconds(1); // wait at least one bit before disabling the driver
		rs485::read_disable::reset();
		rs485::drive_enable::reset();
		xpcc::delayNanoseconds(70); // wait for signal propagation
	}
};

} /* namespace supreme */

#endif /* SUPREME_SENDBUFFER_HPP */
