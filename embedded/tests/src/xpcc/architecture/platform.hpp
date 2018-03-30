#ifndef TEST_XPCC_ARCHITECTURE_PLATFORM_HPP
#define TEST_XPCC_ARCHITECTURE_PLATFORM_HPP


typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;

namespace led {
	namespace red {
		void set(void) {}
		void reset(void) {}
	}
	namespace yellow {
		void set(void) {}
		void reset(void) {}
	}
}

namespace xpcc {
	void delayNanoseconds (unsigned /*d*/) {}
	void delayMicroseconds(unsigned /*d*/) {}
	void delayMilliseconds(unsigned /*d*/) {}
}

namespace rs485 {

	struct {
		unsigned send_enable  = 0;
		unsigned send_disable = 0;
		unsigned recv_enable  = 0;
		unsigned recv_disable = 0;

		void clear() {
			send_enable  = 0;
			send_disable = 0;
			recv_enable  = 0;
			recv_disable = 0;
		}
	} stats;

	namespace drive_enable {
		void set(void) { ++stats.send_enable; }
		void reset(void) { ++stats.send_disable; }
		void setOutput(void) {}
	}
	namespace read_disable {
		void set(void) { ++stats.recv_disable; }
		void reset(void) { ++stats.recv_enable; }
		void setOutput(void) {}
	}
}

#include <vector>
#include <queue>

namespace Uart0 {
	std::vector<uint8_t> recv_buffer; 
	bool buffer_flushed = false;

	std::queue<uint8_t> send_queue;

	void flushWriteBuffer(void) { buffer_flushed = true; }
	void write(unsigned char* input, unsigned len) { 
		//recv_buffer.clear();
		for (unsigned i = 0; i < len; ++i)
			recv_buffer.push_back(input[i]);
	}
	
	bool read(unsigned char& read_byte) { 
		if (send_queue.empty()) return false;
		read_byte = send_queue.front(); 
		send_queue.pop(); 
		return true; 
	}

}


void reset_hardware() {
		Uart0::recv_buffer.clear();
		Uart0::buffer_flushed = false;
		Uart0::send_queue = std::queue<uint8_t>();

		rs485::stats.clear();
}

#endif /* TEST_XPCC_ARCHITECTURE_PLATFORM_HPP */

