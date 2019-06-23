/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_LOWPASS_HPP
#define SUPREME_LOWPASS_HPP

namespace supreme {

/* A computational efficient (IIR) low-pass filtering for measurements. */
template <typename T>
class Lowpass {
	float value;
	const float d0, d1;
public:
	Lowpass(T const& init = T{}, float coeff = .5f) : value(init), d0(coeff), d1(1.f-coeff) { }

	T step(T inval) {
		value = d1 * value + d0 * inval;
		return static_cast<T>(round(value));
	}
};

} /* namespace supreme */

#endif /* SUPREME_LOWPASS_HPP */
