#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "serial.h"

/**
 * @file
 *
 * Tested on ATTINY13A
 *
 * Implementation of serial transmission (8-N-1) at any host supported rate,
 * using Timer1 and an interrupt, allowing the main program to continue
 * running during communication.
 *
 * This is the preferred serial implementation because it has relatively
 * precise control of bit timing.
 * A simple calibration step will tune this delay to the particular chip in use.
 * Jitter in bit times is minimised as the interrupt is timer driven.
 */

// If 1, then the timer interrupt will always be enabled
// If 0, then interrupt is only enabled as long as necessary (during serial transmission)
//       -- this mode is buggy, results in garbled serial output, not clear why (FIXME)
#define INT_ALWAYS 1

enum {
    // States 0..7 indicate the data bit index (0 = LSB) that should
    // define the line level when the next timer interrupt occurs
    STOP_BIT = 8, // Next timer interrupt will set line high
    SENDING_STOP, // Holds off any line change until the stop bit is complete
    IDLE // Indicates transmission is complete and line is ready for new transmission
};

// From ATtiny13A [DATASHEET] page 80,
// Table 11-9. Clock Select Bit Description
enum {
    NO_PRESCALE      = 0b001,
    PRESCALE_BY_8    = 0b010,
    PRESCALE_BY_64   = 0b011,
    PRESCALE_BY_256  = 0b100,
    PRESCALE_BY_1024 = 0b101
};

/**
 * The data byte being transmitted.
 */
static volatile uint8_t data;

/**
 * Represents the next "thing to do" when the timer interrupt
 * is serviced. E.g. state = 0 means "send bit zero (LSB) of
 * current data byte".
 */
static volatile uint8_t state = IDLE;

// Calibration is done by running "chirp" test (serial_timer_delay_test()),
// and taking midpoint of first and last uncorrupted timer values received.

/**
 * Set to correct prescale ratio for the desired baud rate
 * according to calibration test.
 */
uint8_t prescale = PRESCALE_BY_8;

/**
 * Set to timer count for the desired baud rate
 * according to calibration test.
 */
static const uint8_t count = 13; // This works for 128kHz clock with PRESCALE_BY_8

void serial_timer_init() {
    DDRB |= RXMASK;
    TCCR0A = 1 << WGM01; // CTC mode, top is OCR0A; datasheet Table 11-8, page 79
    TCCR0B = prescale;
    OCR0A = count;

    state = IDLE;
    SERIAL_HI();

    TIMSK0 = 1 << OCF0A;

    sei();
}

ISR(TIM0_COMPA_vect) {
    if (state < IDLE) {
        // Transmit the stop bit or next data bit
        if (state >= STOP_BIT || (data & 1)) {
            SERIAL_HI();
        } else {
            SERIAL_LO();
        }

        ++state;
        data >>= 1;
    } else if (! INT_ALWAYS) {
        TIMSK0 &= ~(1 << TOIE0); // transmit done. disable timer interrupt
    }
}

/**
 * Wait until serial communication is finished
 * and line is idle. At this point the timer interrupt
 * will be disabled.
 */
void flush_serial() {
    while (/*(TIMSK & (1 << TOIE1)) &&*/ state != IDLE)
        ;
}

/**
 * Spinwaits until serial line is idle, then begins
 * transmission of `c` as 8 bits. Function returns immediately,
 * does not wait for transmission to occur.
 *
 * TODO: Using this function only allows limited concurrency (after calling sendt(),
 *       main program will block at the next call to sendt(), until first is done).
 *       Need to implement a buffer that can be filled at once and consumed by ISR,
 *       so that the main program continues during the entire buffer transmission.
 */
void sendt(uint8_t c) {
    flush_serial();

    TCNT0 = 0;   // start counting a full bit interval
    SERIAL_LO(); // begin the start bit
    data = c;    // bits of `data` follow,
    state = 0;   // bit zero is next

    if (! INT_ALWAYS) TIMSK0 |= 1 << TOIE0; // enable timer interrupt
}

void sendnum(char marker, uint8_t err) {
    sendt(marker);
    sendt(':');
    sendt('0'+err/100);
    sendt('0'+((err/10)%10));
    sendt('0'+(err%10));
    sendt('\r');
    sendt('\n');
}

void serial_timer_delay_test() {
   // try different values of delay to find the range that works

   for(uint8_t d = 1; d; ++d){
      OCR0A = d; // Update timer interrupt frequency

      sendt('.'); sendt('o'); sendt('O'); sendt('(');
      sendt('0'+((d/100)%10));
      sendt('0'+((d/10)%10));
      sendt('0'+(d%10));
      sendt(')');
      sendt('\r'); sendt('\n');
   }

   OCR0A = count; // reset to configured value
}
