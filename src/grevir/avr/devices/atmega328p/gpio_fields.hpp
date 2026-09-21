#pragma once

#include <grevir/avr/generated/atmega328p/timer_gpio.hpp>
#include <grevir/avr/gpio.hpp>

namespace ardo::sys::avr::arch_atmega328p {
template <typename ByteAccess, typename Barrier>
struct GpioBindings {
  using Binding = base::RegisterSelector<ByteAccess>;
  template <typename Fields, typename Address>
  using Register = base::Register<Fields, Address, Binding::template IoAccessor>;
  using BitsPORTB0 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB0"), bool>, ccPORTB0>;
  using BitsPORTB1 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB1"), bool>, ccPORTB1>;
  using BitsPORTB2 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB2"), bool>, ccPORTB2>;
  using BitsPORTB3 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB3"), bool>, ccPORTB3>;
  using BitsPORTB4 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB4"), bool>, ccPORTB4>;
  using BitsPORTB5 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB5"), bool>, ccPORTB5>;
  using BitsPORTB6 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB6"), bool>, ccPORTB6>;
  using BitsPORTB7 = setl::BitsRW<setl::SemanticType<setl::hash("PORTB7"), bool>, ccPORTB7>;
  using FieldsPORTB = setl::BitFields<BitsPORTB0, BitsPORTB1, BitsPORTB2, BitsPORTB3, BitsPORTB4, BitsPORTB5, BitsPORTB6, BitsPORTB7>;
  using RegisterPORTB = Register<FieldsPORTB, rrPORTB>;
  using BitsDDB0 = setl::BitsRW<setl::SemanticType<setl::hash("DDB0"), bool>, ccDDB0>;
  using BitsDDB1 = setl::BitsRW<setl::SemanticType<setl::hash("DDB1"), bool>, ccDDB1>;
  using BitsDDB2 = setl::BitsRW<setl::SemanticType<setl::hash("DDB2"), bool>, ccDDB2>;
  using BitsDDB3 = setl::BitsRW<setl::SemanticType<setl::hash("DDB3"), bool>, ccDDB3>;
  using BitsDDB4 = setl::BitsRW<setl::SemanticType<setl::hash("DDB4"), bool>, ccDDB4>;
  using BitsDDB5 = setl::BitsRW<setl::SemanticType<setl::hash("DDB5"), bool>, ccDDB5>;
  using BitsDDB6 = setl::BitsRW<setl::SemanticType<setl::hash("DDB6"), bool>, ccDDB6>;
  using BitsDDB7 = setl::BitsRW<setl::SemanticType<setl::hash("DDB7"), bool>, ccDDB7>;
  using FieldsDDB = setl::BitFields<BitsDDB0, BitsDDB1, BitsDDB2, BitsDDB3, BitsDDB4, BitsDDB5, BitsDDB6, BitsDDB7>;
  using RegisterDDB = Register<FieldsDDB, rrDDRB>;
  using BitsPINB0 = setl::BitsRW<setl::SemanticType<setl::hash("PINB0"), bool>, ccPINB0>;
  using BitsPINB1 = setl::BitsRW<setl::SemanticType<setl::hash("PINB1"), bool>, ccPINB1>;
  using BitsPINB2 = setl::BitsRW<setl::SemanticType<setl::hash("PINB2"), bool>, ccPINB2>;
  using BitsPINB3 = setl::BitsRW<setl::SemanticType<setl::hash("PINB3"), bool>, ccPINB3>;
  using BitsPINB4 = setl::BitsRW<setl::SemanticType<setl::hash("PINB4"), bool>, ccPINB4>;
  using BitsPINB5 = setl::BitsRW<setl::SemanticType<setl::hash("PINB5"), bool>, ccPINB5>;
  using BitsPINB6 = setl::BitsRW<setl::SemanticType<setl::hash("PINB6"), bool>, ccPINB6>;
  using BitsPINB7 = setl::BitsRW<setl::SemanticType<setl::hash("PINB7"), bool>, ccPINB7>;
  using FieldsPINB = setl::BitFields<BitsPINB0, BitsPINB1, BitsPINB2, BitsPINB3, BitsPINB4, BitsPINB5, BitsPINB6, BitsPINB7>;
  using RegisterPINB = Register<FieldsPINB, rrPINB>;
  struct ppPB0 : base::GpioPort<ppPB0, base::GpioPortDefinition<
    BitsPORTB0, BitsPINB0, BitsDDB0, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB1 : base::GpioPort<ppPB1, base::GpioPortDefinition<
    BitsPORTB1, BitsPINB1, BitsDDB1, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB2 : base::GpioPort<ppPB2, base::GpioPortDefinition<
    BitsPORTB2, BitsPINB2, BitsDDB2, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB3 : base::GpioPort<ppPB3, base::GpioPortDefinition<
    BitsPORTB3, BitsPINB3, BitsDDB3, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB4 : base::GpioPort<ppPB4, base::GpioPortDefinition<
    BitsPORTB4, BitsPINB4, BitsDDB4, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB5 : base::GpioPort<ppPB5, base::GpioPortDefinition<
    BitsPORTB5, BitsPINB5, BitsDDB5, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB6 : base::GpioPort<ppPB6, base::GpioPortDefinition<
    BitsPORTB6, BitsPINB6, BitsDDB6, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  struct ppPB7 : base::GpioPort<ppPB7, base::GpioPortDefinition<
    BitsPORTB7, BitsPINB7, BitsDDB7, RegisterPORTB, RegisterPINB, RegisterDDB>, Barrier> {};
  using BitsPORTC0 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC0"), bool>, ccPORTC0>;
  using BitsPORTC1 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC1"), bool>, ccPORTC1>;
  using BitsPORTC2 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC2"), bool>, ccPORTC2>;
  using BitsPORTC3 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC3"), bool>, ccPORTC3>;
  using BitsPORTC4 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC4"), bool>, ccPORTC4>;
  using BitsPORTC5 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC5"), bool>, ccPORTC5>;
  using BitsPORTC6 = setl::BitsRW<setl::SemanticType<setl::hash("PORTC6"), bool>, ccPORTC6>;
  using FieldsPORTC = setl::BitFields<BitsPORTC0, BitsPORTC1, BitsPORTC2, BitsPORTC3, BitsPORTC4, BitsPORTC5, BitsPORTC6>;
  using RegisterPORTC = Register<FieldsPORTC, rrPORTC>;
  using BitsDDC0 = setl::BitsRW<setl::SemanticType<setl::hash("DDC0"), bool>, ccDDC0>;
  using BitsDDC1 = setl::BitsRW<setl::SemanticType<setl::hash("DDC1"), bool>, ccDDC1>;
  using BitsDDC2 = setl::BitsRW<setl::SemanticType<setl::hash("DDC2"), bool>, ccDDC2>;
  using BitsDDC3 = setl::BitsRW<setl::SemanticType<setl::hash("DDC3"), bool>, ccDDC3>;
  using BitsDDC4 = setl::BitsRW<setl::SemanticType<setl::hash("DDC4"), bool>, ccDDC4>;
  using BitsDDC5 = setl::BitsRW<setl::SemanticType<setl::hash("DDC5"), bool>, ccDDC5>;
  using BitsDDC6 = setl::BitsRW<setl::SemanticType<setl::hash("DDC6"), bool>, ccDDC6>;
  using FieldsDDC = setl::BitFields<BitsDDC0, BitsDDC1, BitsDDC2, BitsDDC3, BitsDDC4, BitsDDC5, BitsDDC6>;
  using RegisterDDC = Register<FieldsDDC, rrDDRC>;
  using BitsPINC0 = setl::BitsRW<setl::SemanticType<setl::hash("PINC0"), bool>, ccPINC0>;
  using BitsPINC1 = setl::BitsRW<setl::SemanticType<setl::hash("PINC1"), bool>, ccPINC1>;
  using BitsPINC2 = setl::BitsRW<setl::SemanticType<setl::hash("PINC2"), bool>, ccPINC2>;
  using BitsPINC3 = setl::BitsRW<setl::SemanticType<setl::hash("PINC3"), bool>, ccPINC3>;
  using BitsPINC4 = setl::BitsRW<setl::SemanticType<setl::hash("PINC4"), bool>, ccPINC4>;
  using BitsPINC5 = setl::BitsRW<setl::SemanticType<setl::hash("PINC5"), bool>, ccPINC5>;
  using BitsPINC6 = setl::BitsRW<setl::SemanticType<setl::hash("PINC6"), bool>, ccPINC6>;
  using FieldsPINC = setl::BitFields<BitsPINC0, BitsPINC1, BitsPINC2, BitsPINC3, BitsPINC4, BitsPINC5, BitsPINC6>;
  using RegisterPINC = Register<FieldsPINC, rrPINC>;
  struct ppPC0 : base::GpioPort<ppPC0, base::GpioPortDefinition<
    BitsPORTC0, BitsPINC0, BitsDDC0, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC1 : base::GpioPort<ppPC1, base::GpioPortDefinition<
    BitsPORTC1, BitsPINC1, BitsDDC1, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC2 : base::GpioPort<ppPC2, base::GpioPortDefinition<
    BitsPORTC2, BitsPINC2, BitsDDC2, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC3 : base::GpioPort<ppPC3, base::GpioPortDefinition<
    BitsPORTC3, BitsPINC3, BitsDDC3, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC4 : base::GpioPort<ppPC4, base::GpioPortDefinition<
    BitsPORTC4, BitsPINC4, BitsDDC4, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC5 : base::GpioPort<ppPC5, base::GpioPortDefinition<
    BitsPORTC5, BitsPINC5, BitsDDC5, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  struct ppPC6 : base::GpioPort<ppPC6, base::GpioPortDefinition<
    BitsPORTC6, BitsPINC6, BitsDDC6, RegisterPORTC, RegisterPINC, RegisterDDC>, Barrier> {};
  using BitsPORTD0 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD0"), bool>, ccPORTD0>;
  using BitsPORTD1 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD1"), bool>, ccPORTD1>;
  using BitsPORTD2 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD2"), bool>, ccPORTD2>;
  using BitsPORTD3 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD3"), bool>, ccPORTD3>;
  using BitsPORTD4 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD4"), bool>, ccPORTD4>;
  using BitsPORTD5 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD5"), bool>, ccPORTD5>;
  using BitsPORTD6 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD6"), bool>, ccPORTD6>;
  using BitsPORTD7 = setl::BitsRW<setl::SemanticType<setl::hash("PORTD7"), bool>, ccPORTD7>;
  using FieldsPORTD = setl::BitFields<BitsPORTD0, BitsPORTD1, BitsPORTD2, BitsPORTD3, BitsPORTD4, BitsPORTD5, BitsPORTD6, BitsPORTD7>;
  using RegisterPORTD = Register<FieldsPORTD, rrPORTD>;
  using BitsDDD0 = setl::BitsRW<setl::SemanticType<setl::hash("DDD0"), bool>, ccDDD0>;
  using BitsDDD1 = setl::BitsRW<setl::SemanticType<setl::hash("DDD1"), bool>, ccDDD1>;
  using BitsDDD2 = setl::BitsRW<setl::SemanticType<setl::hash("DDD2"), bool>, ccDDD2>;
  using BitsDDD3 = setl::BitsRW<setl::SemanticType<setl::hash("DDD3"), bool>, ccDDD3>;
  using BitsDDD4 = setl::BitsRW<setl::SemanticType<setl::hash("DDD4"), bool>, ccDDD4>;
  using BitsDDD5 = setl::BitsRW<setl::SemanticType<setl::hash("DDD5"), bool>, ccDDD5>;
  using BitsDDD6 = setl::BitsRW<setl::SemanticType<setl::hash("DDD6"), bool>, ccDDD6>;
  using BitsDDD7 = setl::BitsRW<setl::SemanticType<setl::hash("DDD7"), bool>, ccDDD7>;
  using FieldsDDD = setl::BitFields<BitsDDD0, BitsDDD1, BitsDDD2, BitsDDD3, BitsDDD4, BitsDDD5, BitsDDD6, BitsDDD7>;
  using RegisterDDD = Register<FieldsDDD, rrDDRD>;
  using BitsPIND0 = setl::BitsRW<setl::SemanticType<setl::hash("PIND0"), bool>, ccPIND0>;
  using BitsPIND1 = setl::BitsRW<setl::SemanticType<setl::hash("PIND1"), bool>, ccPIND1>;
  using BitsPIND2 = setl::BitsRW<setl::SemanticType<setl::hash("PIND2"), bool>, ccPIND2>;
  using BitsPIND3 = setl::BitsRW<setl::SemanticType<setl::hash("PIND3"), bool>, ccPIND3>;
  using BitsPIND4 = setl::BitsRW<setl::SemanticType<setl::hash("PIND4"), bool>, ccPIND4>;
  using BitsPIND5 = setl::BitsRW<setl::SemanticType<setl::hash("PIND5"), bool>, ccPIND5>;
  using BitsPIND6 = setl::BitsRW<setl::SemanticType<setl::hash("PIND6"), bool>, ccPIND6>;
  using BitsPIND7 = setl::BitsRW<setl::SemanticType<setl::hash("PIND7"), bool>, ccPIND7>;
  using FieldsPIND = setl::BitFields<BitsPIND0, BitsPIND1, BitsPIND2, BitsPIND3, BitsPIND4, BitsPIND5, BitsPIND6, BitsPIND7>;
  using RegisterPIND = Register<FieldsPIND, rrPIND>;
  struct ppPD0 : base::GpioPort<ppPD0, base::GpioPortDefinition<
    BitsPORTD0, BitsPIND0, BitsDDD0, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD1 : base::GpioPort<ppPD1, base::GpioPortDefinition<
    BitsPORTD1, BitsPIND1, BitsDDD1, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD2 : base::GpioPort<ppPD2, base::GpioPortDefinition<
    BitsPORTD2, BitsPIND2, BitsDDD2, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD3 : base::GpioPort<ppPD3, base::GpioPortDefinition<
    BitsPORTD3, BitsPIND3, BitsDDD3, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD4 : base::GpioPort<ppPD4, base::GpioPortDefinition<
    BitsPORTD4, BitsPIND4, BitsDDD4, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD5 : base::GpioPort<ppPD5, base::GpioPortDefinition<
    BitsPORTD5, BitsPIND5, BitsDDD5, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD6 : base::GpioPort<ppPD6, base::GpioPortDefinition<
    BitsPORTD6, BitsPIND6, BitsDDD6, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
  struct ppPD7 : base::GpioPort<ppPD7, base::GpioPortDefinition<
    BitsPORTD7, BitsPIND7, BitsDDD7, RegisterPORTD, RegisterPIND, RegisterDDD>, Barrier> {};
};

} // namespace ardo::sys::avr::arch_atmega328p
