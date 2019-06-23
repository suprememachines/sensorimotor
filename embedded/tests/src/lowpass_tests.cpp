#include "./catch_1.10.0.hpp"
#include <common/lowpass.hpp>

namespace supreme {
namespace local_tests {

TEST_CASE( "lowpass filter smooths out", "[lowpass]")
{
	Lowpass<uint16_t> lowpass;
	/* start with zero */
	REQUIRE(  5 == lowpass.step(10) );
	REQUIRE( 10 == lowpass.step(15) );
	REQUIRE( 30 == lowpass.step(50) );

	for (unsigned i = 0; i < 10; ++i) {
		auto res = lowpass.step(50);
		REQUIRE( 30 < res );
		REQUIRE( res <= 50 );
	}
	REQUIRE( 50 == lowpass.step(50) );
}

TEST_CASE( "no overshoots with lowpass filter on unsigned types", "[lowpass]")
{
	Lowpass<uint16_t> lowpass{0x5555};
	REQUIRE ( 0x5555 == lowpass.step(0x5555) ); // same value is kept

	for (unsigned i = 0; i < 16; ++i)
		REQUIRE ( 0x0 <= lowpass.step(0x0) ); // approach zero
	REQUIRE ( 0x0 == lowpass.step(0x0) );

	for (unsigned i = 0; i < 16; ++i)
		REQUIRE ( 0xFFFF >= lowpass.step(0xFFFF) ); // approach max value
	REQUIRE ( 0xFFFF == lowpass.step(0xFFFF) );
}

}} /* namespace supreme::local_tests */
