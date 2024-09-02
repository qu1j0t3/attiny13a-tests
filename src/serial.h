#ifndef SERIAL_H_
#define SERIAL_H_

#include <avr/io.h>

#define RXMASK 0b10000 // PB4 is connected to the host RX pin

static inline void SERIAL_LO() {
    PORTB &= ~RXMASK;
}

static inline void SERIAL_HI() {
    PORTB |= RXMASK;
}

void serial_timer_init();
void serial_timer_delay_test();
void sendt(uint8_t b);
void sendnum(char marker, uint8_t err);
void flush_serial();

void TOGGLE_LED();

#endif // SERIAL_H_
