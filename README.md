# sensorimotor
Sensorimotor is a fully hackable, free-design hardware and free software smart servo controller.

It is developed within the scope of my thesis and is optimized for the application in
walking robots. However, a variety of other projects might benefit from it, too.

It comprises a fully integrated 6 A H-bridge (Infineon IFX9201SG) for
driving DC motors, a failsafe RS485 driver (LTC2850) for reliable communication and an Atmel
ATMEGA328P microcontroller for the computations. 

It has an onboard current sensor (ZXCT1022), a battery voltage sensing, reverse polarity protection, transient voltage
suppression, and back-emf voltage sensing. The circuit board provides connections for
an external potentiometer (usually used as servo position sensor), temperature sensor and can be extended with other devices connected via I2C.

The microcontroller runs at 16 MHz (ceramic resonator) and is dedicated to sending and receiving communication packages, computing motor control loops, setting
pulse-width modulated (PWM) values and reading analog and external sensor inputs.

Firmware is flashed with a simple standard 6-pin in-system programming adapter (ISP). The integrated H-bridge has higher-level features such as internal overtemperature protection and open load detection (if no motor is connected). 

The input voltage range of the sensorimotor is 5–12.6 V. A simple low-dropout linear regulator (MIC5225) provides the necessary 3V3 for the microcontroller and the other digital components. The input voltage is passed unregulated to the power electronics (and thus the motor). If the used motor is restricted to less than the input
voltage, the duty cycle of the motor output can be used to reduce the average voltage accordingly. The full schematics can be found in this repository.

The board layout of the sensorimotor is designed to fit roughly into large-scale servos of different manufacturers. The SMT parts are restricted to be on a single side of the board to reduce manufacturing costs or allow for easier soldering. The power traces are made as wide as board space was available to reduce trace resistance and increase the heat sink. However, an additional heat sink should be considered when driving heavier loads.
