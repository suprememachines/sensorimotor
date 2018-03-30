
#include "sensorimotor.hpp"

xpcc::IODeviceWrapper< Uart0, xpcc::IOBuffer::BlockIfFull > serialDevice;
xpcc::IOStream serialStream(serialDevice);
