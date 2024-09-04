#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
//#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>

#include "serial.h"

int main() {
  // disable Clock Prescale (system clock should be full 9.6MHz)
  // see datasheet page 34, 6.4.2 CLKPR - Clock Prescale Register
  CLKPR = 1 << CLKPCE;
  CLKPR = 0; // No prescaling - full speed clock

  serial_timer_init();

  _delay_ms(10);

  //serial_timer_delay_test();

while(1){
  sendt('O');
  sendt('K');
  sendt('.');
  sendt('\r');
  sendt('\n');
  _delay_ms(500);
}

  return EXIT_SUCCESS;
}
