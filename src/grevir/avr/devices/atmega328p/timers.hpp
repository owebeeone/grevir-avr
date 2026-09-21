#pragma once

#include <grevir/avr/devices/atmega328p/timer_registers.hpp>
#include <grevir/avr/devices/atmega328p/gpio_fields.hpp>
#include <grevir/avr/timer/output.hpp>

namespace ardo::sys::avr::arch_atmega328p {
using base::TimerDefinition;
using base::OutputCompare;
using base::TimerCapture;
template <typename ByteAccess, typename Barrier>
struct TimerBindings : TimerRegisters<ByteAccess, Barrier>, GpioBindings<ByteAccess, Barrier> {
  using RegisterBindings = TimerRegisters<ByteAccess, Barrier>;
  using Gpio = GpioBindings<ByteAccess, Barrier>;
  using RegisterIRC1 = typename RegisterBindings::RegisterIRC1;
  using RegisterOCR0A = typename RegisterBindings::RegisterOCR0A;
  using RegisterOCR0B = typename RegisterBindings::RegisterOCR0B;
  using RegisterOCR1A = typename RegisterBindings::RegisterOCR1A;
  using RegisterOCR1B = typename RegisterBindings::RegisterOCR1B;
  using RegisterOCR2A = typename RegisterBindings::RegisterOCR2A;
  using RegisterOCR2B = typename RegisterBindings::RegisterOCR2B;
  using RegisterTCCR0A = typename RegisterBindings::RegisterTCCR0A;
  using RegisterTCCR0AB = typename RegisterBindings::RegisterTCCR0AB;
  using RegisterTCCR0B = typename RegisterBindings::RegisterTCCR0B;
  using RegisterTCCR1A = typename RegisterBindings::RegisterTCCR1A;
  using RegisterTCCR1AB = typename RegisterBindings::RegisterTCCR1AB;
  using RegisterTCCR1B = typename RegisterBindings::RegisterTCCR1B;
  using RegisterTCCR1C = typename RegisterBindings::RegisterTCCR1C;
  using RegisterTCCR2A = typename RegisterBindings::RegisterTCCR2A;
  using RegisterTCCR2AB = typename RegisterBindings::RegisterTCCR2AB;
  using RegisterTCCR2B = typename RegisterBindings::RegisterTCCR2B;
  using RegisterTCNT0 = typename RegisterBindings::RegisterTCNT0;
  using RegisterTCNT1 = typename RegisterBindings::RegisterTCNT1;
  using RegisterTCNT2 = typename RegisterBindings::RegisterTCNT2;
  using RegisterTIFR0 = typename RegisterBindings::RegisterTIFR0;
  using RegisterTIFR1 = typename RegisterBindings::RegisterTIFR1;
  using RegisterTIFR2 = typename RegisterBindings::RegisterTIFR2;
  using RegisterTIMSK0 = typename RegisterBindings::RegisterTIMSK0;
  using RegisterTIMSK1 = typename RegisterBindings::RegisterTIMSK1;
  using RegisterTIMSK2 = typename RegisterBindings::RegisterTIMSK2;
  using ppPB1 = typename Gpio::ppPB1;
  using ppPB2 = typename Gpio::ppPB2;
  using ppPB3 = typename Gpio::ppPB3;
  using ppPD3 = typename Gpio::ppPD3;
  using ppPD5 = typename Gpio::ppPD5;
  using ppPD6 = typename Gpio::ppPD6;
using Timer0Def = TimerDefinition<
  BitsWGM0_210,
  BitsCS01_16,
  BitsFOC0A,
  BitsFOC0B,
  BitsTCNT0,
  std::tuple<
    OutputCompare<BitsOCR0A, BitsOCIE0A, BitsOCF0A, BitsCOM0A, BitsCOM0A8, ppPD6>,
    OutputCompare<BitsOCR0B, BitsOCIE0B, BitsOCF0B, BitsCOM0B, BitsCOM0B8, ppPD5>>,
  TimerCapture<void, void, void>,
  std::tuple<
    RegisterTCCR0AB,
    RegisterTCCR0A,
    RegisterTCCR0B,
    RegisterTCNT0,
    RegisterOCR0A,
    RegisterOCR0B,
    RegisterTIMSK0,
    RegisterTIFR0>,
  setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra>, WgmEnumTraits<EnumWGM0>
>;

using Timer1Def = TimerDefinition<
  BitsWGM1_3210,
  BitsCS11_16,
  BitsFOC1A,
  BitsFOC1B,
  BitsTCNT1,
  std::tuple<
    OutputCompare<BitsOCR1A, BitsOCIE1A, BitsOCF1A, BitsCOM1A, BitsCOM1A8, ppPB1>,
    OutputCompare<BitsOCR1B, BitsOCIE1B, BitsOCF1B, BitsCOM1B, BitsCOM1B8, ppPB2>>,
  TimerCapture<BitsICR1, BitsICNC1_16, BitsICES1_16>,
  std::tuple<
    RegisterTCCR1A,
    RegisterTCCR1B,
    RegisterTCCR1AB,
    RegisterTCCR1C,  // Timer1 has FOC1A/B in this register.
    RegisterIRC1,
    RegisterTCNT1,
    RegisterOCR1A,
    RegisterOCR1B,
    RegisterTIMSK1,
    RegisterTIFR1>,
  setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra>, WgmEnumTraits<EnumWGM1>
>;

using Timer2Def = TimerDefinition<
  BitsWGM2_210,
  BitsCS21_16,
  BitsFOC2A,
  BitsFOC2B,
  BitsTCNT2,
  std::tuple<
    OutputCompare<BitsOCR2A, BitsOCIE2A, BitsOCF2A, BitsCOM2A, BitsCOM2A8, ppPB3>,
    OutputCompare<BitsOCR2B, BitsOCIE2B, BitsOCF2B, BitsCOM2B, BitsCOM2B8, ppPD3>>,
  TimerCapture<void, void, void>,
  std::tuple<
    RegisterTCCR2AB,
    RegisterTCCR2A,
    RegisterTCCR2B,
    RegisterTCNT2,
    RegisterOCR2A,
    RegisterOCR2B,
    RegisterTIMSK2,
    RegisterTIFR2>,
  setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra>, WgmEnumTraits<EnumWGM0>
>;


  using Timer0 = base::Timer<Timer0Def, TccrEnumTraits<EnumCS1>>;
  using Timer1 = base::Timer<Timer1Def, TccrEnumTraits<EnumCS1>>;
  using Timer2 = base::Timer<Timer2Def, TccrEnumTraits<EnumCS2>>;
  using Timers = std::tuple<Timer0, Timer1, Timer2>;
};

} // namespace ardo::sys::avr::arch_atmega328p
