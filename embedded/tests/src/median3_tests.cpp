#include "./catch_1.10.0.hpp"
#include <common/median3.hpp>

namespace supreme {
namespace local_tests {

TEST_CASE( "median of 3 function", "[math]")
{
	REQUIRE( 2 == median_of_3(1,2,3) );
	REQUIRE( 2 == median_of_3(2,3,1) );
	REQUIRE( 2 == median_of_3(3,1,2) );

	REQUIRE( -10 == median_of_3(-1,-10,-100) );

	REQUIRE( 1.0f == median_of_3(1.0f, 23.42f,-13.37f) );
}

TEST_CASE( "median of 3 class", "[math]")
{
	Median3<uint8_t> u;
	Median3<float> f;

	REQUIRE( 0 == u.step(  1) );
	REQUIRE( 1 == u.step(  1) );
	REQUIRE( 1 == u.step(255) ); // big value (outlier)
	REQUIRE( 2 == u.step(  2) );

	REQUIRE( 0.0f == f.step(   1.0f) );
	REQUIRE( 1.0f == f.step(   1.0f) );
	REQUIRE( 1.0f == f.step(1000.0f) ); // big value (outlier)
	REQUIRE( 2.0f == f.step(   2.0f) );
}

}} /* namespace supreme::local_tests */
