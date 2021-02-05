/*---------------------------------+
 | Jetpack Cognition Lab, Inc.     |
 | Supreme Machines                |
 | Sensorimotor Firmware           |
 | Matthias Kubisch                |
 | kubisch@informatik.hu-berlin.de |
 | January 15th 2021               |
 +---------------------------------*/

#ifndef SUPREME_BOOTLOADER_HPP
#define SUPREME_BOOTLOADER_HPP



namespace supreme {

constexpr uint16_t BOOTADDR = 0x3e00;

typedef void (*jump_t)(void);
void jump(uint16_t addr)
{
	jump_t func = (jump_t) addr;
	func();
}

void start_bootloader(void) { jump(BOOTADDR); }

} /* namespace supreme */

#endif /* SUPREME_BOOTLOADER_HPP */
