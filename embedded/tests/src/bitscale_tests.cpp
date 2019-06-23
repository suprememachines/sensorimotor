#include "./catch_1.10.0.hpp"
#include <common/bitscale.hpp>

namespace supreme {
namespace local_tests {

void printbits(uint16_t n) {
	while (n) {
	    if (n & 1) printf("1");
	    else printf("0");
	    n >>= 1;
	}
	printf("\n");
}

/* requirements for promoting bits:
   transport: 0xFF to 0x3FF  (255 -> 1023) max. values
              0x00 to 0x000  (0   -> 0   ) min. values
              0x7F to 0x1FF  (127 -> 511 ) center
   keeping linearity
*/

TEST_CASE( "scaling preserves max. value", "[bitscale]")
{
	uint8_t val8 = 255;
	uint16_t val10 = promote_N<2>( val8 );
	REQUIRE( val10 == 1023 );

	uint16_t val12 = promote_N<4>( val8 );
	REQUIRE( val12 == 4095 );

	uint16_t val16 = promote_N<8>( val8 );
	REQUIRE( val16 == 0xFFFF );
}

TEST_CASE( "scaling preserves min. value", "[bitscale]")
{
	uint8_t val8 = 0;
	uint16_t val10 = promote_N<2>( val8 );
	REQUIRE( val10 == 0 );

	uint16_t val12 = promote_N<4>( val8 );
	REQUIRE( val12 == 0 );

	uint16_t val16 = promote_N<8>( val8 );
	REQUIRE( val16 == 0 );
}

TEST_CASE( "scaling preserves center", "[bitscale]")
{
	uint8_t val8 = 127;
	uint16_t val10 = promote_N<2>( val8 );
	REQUIRE( val10 == 511 );

	uint16_t val12 = promote_N<4>( val8 );
	REQUIRE( val12 == 2047 );

	uint16_t val16 = promote_N<8>( val8 );
	REQUIRE( val16 == 0x7FFF );
}

TEST_CASE( "scaling almost preserves linearity", "[bitscale]")
{
	constexpr float sf = 1023.f / 255.f;

	for (unsigned i = 0; i < 256; ++i)
	{
		uint8_t val8 = i;
		uint16_t val10 = promote_N<2>( val8 );
		uint16_t check = static_cast<uint16_t>(round(sf * val8));
		//printf("%u -> %4u =?= %4u (%7.2f) \t", val8, val10, check, sf * val8);
		//printbits(val10);
		REQUIRE( (val10 - check) <= 3 );
	}
}

}} /* namespace supreme::local_tests */
