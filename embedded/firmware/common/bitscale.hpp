/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_BITSCALE_HPP
#define SUPREME_BITSCALE_HPP

namespace supreme {

template <int N> inline uint16_t promote_N(uint8_t val) { return val ? ((val + 1) << N) - 1 : 0; }

} /* namespace supreme */

#endif /* SUPREME_BITSCALE_HPP */
