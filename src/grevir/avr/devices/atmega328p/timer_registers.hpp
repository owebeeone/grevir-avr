#pragma once

#include <grevir/avr/devices/atmega328p/timer_fields.hpp>
#include <grevir/avr/devices/atmega328p/timer_access.hpp>

namespace ardo::sys::avr::arch_atmega328p {
// ByteAccess performs byte IO; Barrier supplies nestable RAII interrupt exclusion.
template <typename ByteAccess, typename Barrier>
struct TimerRegisters {
  using Access = nfp::TimerAccess<ByteAccess, Barrier>;
  using Binding = base::RegisterSelector<Access>;
  template <typename Fields, typename Address>
  using Register = base::Register<Fields, Address, Binding::template IoAccessor>;
  using RegisterTCCR0A = Register<FieldsTCCR0A, rrTCCR0A>;
  using RegisterTCCR0B = Register<FieldsTCCR0B, rrTCCR0B>;
  using RegisterTCCR0AB = Register<FieldsTCCR0AB, rrTCCR0AB>;
  using RegisterTCNT0 = Register<FieldsTCNT0, rrTCNT0>;
  using RegisterOCR0A = Register<FieldsOCR0A, rrOCR0A>;
  using RegisterOCR0B = Register<FieldsOCR0B, rrOCR0B>;
  using RegisterTIMSK0 = Register<FieldsTIMSK0, rrTIMSK0>;
  using RegisterTIFR0 = Register<FieldsTIFR0, rrTIFR0>;
  using RegisterTCCR2A = Register<FieldsTCCR2A, rrTCCR2A>;
  using RegisterTCCR2B = Register<FieldsTCCR2B, rrTCCR2B>;
  using RegisterTCCR2AB = Register<FieldsTCCR2AB, rrTCCR2AB>;
  using RegisterTCNT2 = Register<FieldsTCNT2, rrTCNT2>;
  using RegisterOCR2A = Register<FieldsOCR2A, rrOCR2A>;
  using RegisterOCR2B = Register<FieldsOCR2B, rrOCR2B>;
  using RegisterTIMSK2 = Register<FieldsTIMSK2, rrTIMSK2>;
  using RegisterTIFR2 = Register<FieldsTIFR2, rrTIFR2>;
  using RegisterASSR = Register<FieldsASSR, rrASSR>;
  using RegisterGTCCR = Register<FieldsGTCCR, rrGTCCR>;
  using RegisterTCCR1A = Register<FieldsTCCR1A, rrTCCR1A>;
  using RegisterTCCR1B = Register<FieldsTCCR1B, rrTCCR1B>;
  using RegisterTCCR1AB = Register<FieldsTCCR1AB, rrTCCR1AB>;
  using RegisterTCCR1C = Register<FieldsTCCR1C, rrTCCR1C>;
  using RegisterTIMSK1 = Register<FieldsTIMSK1, rrTIMSK1>;
  using RegisterTIFR1 = Register<FieldsTIFR1, rrTIFR1>;
  using RegisterTCNT1 = Register<FieldsTCNT1, rrTCNT1>;
  using RegisterOCR1A = Register<FieldsOCR1A, rrOCR1A>;
  using RegisterOCR1B = Register<FieldsOCR1B, rrOCR1B>;
  using RegisterIRC1 = Register<FieldsICR1, rrICR1>;
  using RegisterICR1 = RegisterIRC1; // Preserve the legacy misspelled alias too.
};

} // namespace ardo::sys::avr::arch_atmega328p
