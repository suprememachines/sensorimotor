#ifndef SUPREME_MOTOR_IFX9201SG_HPP
#define SUPREME_MOTOR_IFX9201SG_HPP

/* motor_ifx9201sg.hpp */
#include <xpcc/architecture/platform.hpp>

namespace supreme {

class motor_ifx9201sg {
public:

    motor_ifx9201sg()
    {
        /* setup motor bridge */
        disable();           // motor bride disabled by default
        motor::VSO::set();   // enable motor bridge logic
        motor::DIR::set();   // set direction
        motor::PWM::reset(); // disable pwm

        /* setup pwm */
        TCCR1A = (1<<WGM10) | (1<<COM1A1); // OC1A and OC1B are cleared on compare match
        TCCR1B = (1<<WGM12)                // Wave Form Generation is Fast PWM 8 Bit,
               | (1<<CS11);                // set prescaler to 8 -> 7812,5 Hz

        OCR1A = 0; // set pwm to zero duty cycle

        /* TODO: set up a second pwm on the disable pin and synchronize the duty_cycle_shares
         * TODO: test if disabling the h-bridge does change the motors behavior, in order to
         *       utilize this as a second mode.
         *
         *    +----+          +----+
         *    | on | freerun  | on | freerun
         *    +    +----------+    +----------+
         *              +-----+         +-----+
         *              | dis |         | dis |     TODO: can this be used to break?
         *    ----------+     +---------+     +-----
         */
    }

    void toggle_direction() { motor::DIR::toggle(); }
    void set_dir(bool dir) {
        if (dir) motor::DIR::set();
        else motor::DIR::reset();
    }

    void set_pwm(uint8_t dc) { OCR1A = dc; }

    void enable() { motor::DIS::reset(); }
    void disable() { motor::DIS::set(); }
};

} /* namespace supreme */

#endif /* SUPREME_MOTOR_IFX9201SG_HPP */
