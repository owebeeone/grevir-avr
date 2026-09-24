#pragma once

#if defined(__AVR_ATmega328P__)
#include <avr/io.h>
#include <avr/interrupt.h>

#define GREVIR_ATMEGA328P_TIMER1_OVERFLOW_VECTOR TIMER1_OVF_vect

namespace grevir::avr::atmega328p {

struct Timer1Overflow {
  static void mask() noexcept { TIMSK1 &= static_cast<unsigned char>(~_BV(TOIE1)); }
  static void enable_preserving_pending() noexcept { TIMSK1 |= _BV(TOIE1); }
  static bool pending() noexcept { return (TIFR1 & _BV(TOV1)) != 0; }
};

} // namespace grevir::avr::atmega328p
#endif
