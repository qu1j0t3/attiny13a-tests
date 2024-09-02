#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
//#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>

#include "serial.h"

int main() {
  CLKPR = (1 << CLKPCE); // disable Clock Prescale (system clock should be full 9.6MHz)

  serial_timer_init();

  _delay_ms(10);

while(1){
  sendt('O');
  sendt('K');
  sendt('.');
  sendt('\r');
  sendt('\n');
  _delay_ms(500);
}

while(1){
  extern uint8_t prescale;

// From ATtiny13A [DATASHEET] page 80,
// Table 11-9. Clock Select Bit Description
enum {
    NO_PRESCALE      = 0b001,
    PRESCALE_BY_8    = 0b010,
    PRESCALE_BY_64   = 0b011,
    PRESCALE_BY_256  = 0b100,
    PRESCALE_BY_1024 = 0b101
};

  serial_timer_delay_test();
  switch(prescale) {
    case NO_PRESCALE: prescale = PRESCALE_BY_8; break;
    case PRESCALE_BY_8: prescale = PRESCALE_BY_64; break;
    case PRESCALE_BY_64: prescale = PRESCALE_BY_256; break;
    case PRESCALE_BY_256: prescale = NO_PRESCALE; break;
  }
  _delay_ms(10);
}

  return EXIT_SUCCESS;
}
