/*
 Supreme Temperature Sensor

 30.05.2017 originally written by m.marmulla
 10.11.2017 revised by m.kubisch


  Vref---R---+---Rt---GND
             |
             |
            Vadc


  Vadc      Rt
  ---- = --------
  Vref   (R + Rt)

  Rt = Vx * R / (1 - Vx), mit Vx = Vadc/Vref


  Verhältnis zwischen R (Ohm) und Rt (Ohm) in voltagex = Vadc/Vref
  RT dann in float resistorT (in Ohm)
  Zusammenhang von Temperatur und Widerstand (wir verwenden den 110)
  mit den werten aus https://www.nxp.com/documents/data_sheet/KTY81_SER.pdf
  (Seite 4)

  linear:
    Temp(RT)= (Temp2 - Temp1) / (R2 - R1) * (Rt - R1) + Temp1

  quadratisch
    Temp(RT) = -0.0000259748 * RT^2 + 0.185103 * RT - 136.115;
    der Theorieteil dazu befindet sich in tempsensorfit3.nb
    oder Martin fragen :)

  optimaler Widerstand R: (bitte messen und richtigen Wert ergänzen)

  http://www.wolframalpha.com/input/?i=Plot((5*(r%2B1.5)-7.5*(r%2B1))%2F((r%2B1)(r%2B1.5)))+from+0+to+2
  http://www.wolframalpha.com/input/?i=Solve+(+D(+(5*(r%2B1.5)-7.5*(r%2B1))+%2F+((r%2B1)(r%2B1.5))+,r)+%3D+0+)

 */

namespace supreme {

namespace constants {
	const int C1 = 55;
	const int C0 = -25450;
}

/*
 25: 512
 80: 612.7
*/

inline int16_t get_temperature_celsius(uint16_t v_adc)
{
	if (v_adc >= 1023) return 0xffff; /* no sensor connected */
	return (constants::C1 * v_adc + constants::C0); /* value in 0.01 degree celsius */
}

} /* namespace supreme */
