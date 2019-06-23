/*---------------------------------+
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | November 2018                   |
 +---------------------------------*/

#ifndef SUPREME_MEDIAN3_HPP
#define SUPREME_MEDIAN3_HPP

namespace supreme {

template <typename T> inline T median_of_3(T a, T b, T c);

/* computes median-of-three of a given variable*/
template <typename T>
class Median3 {
	T val_1, val_2;
public:
	Median3() : val_1(), val_2() {}

	T step(T val_0) {
		const T result = median_of_3(val_0, val_1, val_2);
		val_2 = val_1;
		val_1 = val_0;
		return result;
	}

};

template <typename T>
inline T median_of_3(T a, T b, T c)
{
	if (a > b)
		if (b > c)
			return b;
		else
			return (a > c) ? c : a;
	else
		if (b < c)
			return b;
		else
			return (a > c) ? a : c;
}

} /* namespace supreme */

#endif /* SUPREME_MEDIAN3_HPP */
