#pragma once

#include <grevir/avr/generated/atmega328p/timer_gpio.hpp>
#include <grevir/avr/devices/atmega328p/timer_modes.hpp>
#include <grevir/avr/register.hpp>

namespace ardo::sys::avr::arch_atmega328p {
using setl::NA;
enum class EnumCOMn : unsigned char {
  disconnect = 0b00,
  toggle = 0b01, // Reserved for Fast PWM mode on COM2A
  clear = 0b10,
  set = 0b11
};

/**
 * Bit field definitions for the COMnA and COMnB fields.
 */
using BitsCOM0A = setl::BitsRW<
  setl::SemanticType<setl::hash("COM0A"), EnumCOMn>, ccCOM0A1, ccCOM0A0>;
using BitsCOM0B = setl::BitsRW<
  setl::SemanticType<setl::hash("COM0B"), EnumCOMn>, ccCOM0B1, ccCOM0B0>;
using BitsCOM1A = setl::BitsRW<
  setl::SemanticType<setl::hash("COM1A"), EnumCOMn>, ccCOM1A1, ccCOM1A0>;
using BitsCOM1B = setl::BitsRW<
  setl::SemanticType<setl::hash("COM1B"), EnumCOMn>, ccCOM1B1, ccCOM1B0>;
using BitsCOM2A = setl::BitsRW<
  setl::SemanticType<setl::hash("COM2A"), EnumCOMn>, ccCOM2A1, ccCOM2A0>;
using BitsCOM2B = setl::BitsRW<
  setl::SemanticType<setl::hash("COM2B"), EnumCOMn>, ccCOM2B1, ccCOM2B0>;

/**
 * Bit field defintions for distinguishing access to the COMn bits from
 * 8 bit register defintions.
 */
using BitsCOM0A8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM0A8"), EnumCOMn>, ccCOM0A1, ccCOM0A0>;
using BitsCOM0B8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM0B8"), EnumCOMn>, ccCOM0B1, ccCOM0B0>;
using BitsCOM1A8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM1A8"), EnumCOMn>, ccCOM1A1, ccCOM1A0>;
using BitsCOM1B8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM1B8"), EnumCOMn>, ccCOM1B1, ccCOM1B0>;
using BitsCOM2A8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM2A8"), EnumCOMn>, ccCOM2A1, ccCOM2A0>;
using BitsCOM2B8 = setl::BitsRW<
  setl::SemanticType<setl::hash("COM2B8"), EnumCOMn>, ccCOM2B1, ccCOM2B0>;


// Define registers for timer/counter 0
using BitsWGM0_10 = setl::BitsRW<EnumWGM0, NA, ccWGM01, ccWGM00>;
using BitsWGM0_2 = setl::BitsRW<EnumWGM0, ccWGM02, NA, NA>;
// Define bitfields when treating TCCR0A + TCCR0B as a 16 bit value.
using BitsWGM0_210 = setl::BitsRW<EnumWGM0, ccWGM02 + 8, ccWGM01, ccWGM00>;

using BitsCS01 = setl::BitsRW<EnumCS0, ccCS02, ccCS01, ccCS00>;
// Define bitfields when treating TCCR0A + TCCR0B as a 16 bit value.
using BitsCS01_16 = setl::BitsRW<EnumCS0, ccCS02 + 8, ccCS01 + 8, ccCS00 + 8>;

// Define force output compare bits for timer/counter 0.
using BitsFOC0A = setl::BitsRW<setl::SemanticType<setl::hash("FOC0A"), bool>, ccFOC0A>;
using BitsFOC0B = setl::BitsRW<setl::SemanticType<setl::hash("FOC0B"), bool>, ccFOC0B>;
// Define 16 bit register modes for these bits.
using BitsFOC0A_16 = setl::BitsRW<setl::SemanticType<setl::hash("FOC0A"), bool>, ccFOC0A + 8>;
using BitsFOC0B_16 = setl::BitsRW<setl::SemanticType<setl::hash("FOC0B"), bool>, ccFOC0B + 8>;

// Define register TCCR0A.
using FieldsTCCR0A = setl::BitFields<BitsCOM0A8, BitsCOM0B8, BitsWGM0_10>;

// Define register TCCR0B.
using FieldsTCCR0B = setl::BitFields<BitsFOC0A, BitsFOC0B, BitsWGM0_2, BitsCS01>;

// Define register TCCR0AB.
using FieldsTCCR0AB = setl::BitFields<
  BitsCOM0A, BitsCOM0B,
  BitsWGM0_210,
  BitsFOC0A_16, BitsFOC0B_16,
  BitsCS01_16>;
using rrTCCR0AB = rrTCCR0A::ForType<std::uint16_t>;

using BitsTCNT0 = setl::BitsRW<setl::SemanticType<setl::hash("TCNT0"), std::uint8_t>,
  ccTCNT0_7,
  ccTCNT0_6,
  ccTCNT0_5,
  ccTCNT0_4,
  ccTCNT0_3,
  ccTCNT0_2,
  ccTCNT0_1,
  ccTCNT0_0
>;
using FieldsTCNT0 = setl::BitFields<BitsTCNT0>;

using BitsOCR0A = setl::BitsRW<setl::SemanticType<setl::hash("OCR0A"), std::uint8_t>,
  ccOCR0A_7,
  ccOCR0A_6,
  ccOCR0A_5,
  ccOCR0A_4,
  ccOCR0A_3,
  ccOCR0A_2,
  ccOCR0A_1,
  ccOCR0A_0
>;
using FieldsOCR0A = setl::BitFields<BitsOCR0A>;

using BitsOCR0B = setl::BitsRW<setl::SemanticType<setl::hash("OCR0B"), std::uint8_t>,
  ccOCR0B_7,
  ccOCR0B_6,
  ccOCR0B_5,
  ccOCR0B_4,
  ccOCR0B_3,
  ccOCR0B_2,
  ccOCR0B_1,
  ccOCR0B_0
>;
using FieldsOCR0B = setl::BitFields<BitsOCR0B>;

// Timer/Counter0 Interrupt Mask Register.
using BitsOCIE0B = setl::BitsRW<setl::SemanticType<setl::hash("OCIE0B"), bool>, ccOCIE0B>;
using BitsOCIE0A = setl::BitsRW<setl::SemanticType<setl::hash("OCIE0A"), bool>, ccOCIE0A>;
using BitsTOIE0 = setl::BitsRW<setl::SemanticType<setl::hash("TOIE0"), bool>, ccTOIE0>;
using FieldsTIMSK0 = setl::BitFields<BitsOCIE0B, BitsOCIE0A, BitsTOIE0>;

// Timer/Counter0 Interrupt Flag Register.
using BitsOCF0B = setl::BitsRW<setl::SemanticType<setl::hash("OCF0B"), bool>, ccOCF0B>;
using BitsOCF0A = setl::BitsRW<setl::SemanticType<setl::hash("OCF0A"), bool>, ccOCF0A>;
using BitsTOV0 = setl::BitsRW<setl::SemanticType<setl::hash("TOV0"), bool>, ccTOV0>;
using FieldsTIFR0 = setl::BitFields<BitsOCF0B, BitsOCF0A, BitsTOV0>;

// Define registers for timer/counter 2
using BitsWGM2_10 = setl::BitsRW<EnumWGM2, NA, ccWGM21, ccWGM20>;
using BitsWGM2_2 = setl::BitsRW<EnumWGM2, ccWGM22, NA, NA>;
// Define bitfields when treating TCCR2A + TCCR2B as a 16 bit value.
using BitsWGM2_210 = setl::BitsRW<EnumWGM2, ccWGM22 + 8, ccWGM21, ccWGM20>;

using BitsCS21 = setl::BitsRW<EnumCS2, ccCS22, ccCS21, ccCS20>;
// Define bitfields when treating TCCR2A + TCCR2B as a 16 bit value.
using BitsCS21_16 = setl::BitsRW<EnumCS2, ccCS22 + 8, ccCS21 + 8, ccCS20 + 8>;

// Define force output compare bits for timer/counter 2.
using BitsFOC2A = setl::BitsRW<setl::SemanticType<setl::hash("FOC2A"), bool>, ccFOC2A>;
using BitsFOC2B = setl::BitsRW<setl::SemanticType<setl::hash("FOC2B"), bool>, ccFOC2B>;
// Define 16 bit register modes for these bits.
using BitsFOC2A_16 = setl::BitsRW<setl::SemanticType<setl::hash("FOC2A"), bool>, ccFOC2A + 8>;
using BitsFOC2B_16 = setl::BitsRW<setl::SemanticType<setl::hash("FOC2B"), bool>, ccFOC2B + 8>;

// Define register TCCR2A.
using FieldsTCCR2A = setl::BitFields<BitsCOM2A8, BitsCOM2B8, BitsWGM2_10>;

// Define register TCCR0B.
using FieldsTCCR2B = setl::BitFields<BitsFOC2A, BitsFOC2B, BitsWGM2_2, BitsCS21>;

// Define register TCCR0AB.
using FieldsTCCR2AB = setl::BitFields<
  BitsCOM2A, BitsCOM2B,
  BitsWGM2_210,
  BitsFOC2A_16, BitsFOC2B_16,
  BitsCS21_16>;
using rrTCCR2AB = rrTCCR2A::ForType<std::uint16_t>;

using BitsTCNT2 = setl::BitsRW<setl::SemanticType<setl::hash("TCNT2"), std::uint8_t>,
  ccTCNT2_7,
  ccTCNT2_6,
  ccTCNT2_5,
  ccTCNT2_4,
  ccTCNT2_3,
  ccTCNT2_2,
  ccTCNT2_1,
  ccTCNT2_0
>;
using FieldsTCNT2 = setl::BitFields<BitsTCNT2>;

using BitsOCR2A = setl::BitsRW<setl::SemanticType<setl::hash("OCR2A"), std::uint8_t>,
  ccOCR2_7,
  ccOCR2_6,
  ccOCR2_5,
  ccOCR2_4,
  ccOCR2_3,
  ccOCR2_2,
  ccOCR2_1,
  ccOCR2_0
>;
using FieldsOCR2A = setl::BitFields<BitsOCR2A>;

using BitsOCR2B = setl::BitsRW<setl::SemanticType<setl::hash("OCR2B"), std::uint8_t>,
  ccOCR2_7,
  ccOCR2_6,
  ccOCR2_5,
  ccOCR2_4,
  ccOCR2_3,
  ccOCR2_2,
  ccOCR2_1,
  ccOCR2_0
>;
using FieldsOCR2B = setl::BitFields<BitsOCR2B>;

// Timer/Counter2 Interrupt Mask Register.
using BitsOCIE2B = setl::BitsRW<setl::SemanticType<setl::hash("OCIE2B"), bool>, ccOCIE2B>;
using BitsOCIE2A = setl::BitsRW<setl::SemanticType<setl::hash("OCIE2A"), bool>, ccOCIE2A>;
using BitsTOIE2 = setl::BitsRW<setl::SemanticType<setl::hash("TOIE2"), bool>, ccTOIE2>;
using FieldsTIMSK2 = setl::BitFields<BitsOCIE2B, BitsOCIE2A, BitsTOIE2>;

// Timer/Counter2 Interrupt Flag Register.
using BitsOCF2B = setl::BitsRW<setl::SemanticType<setl::hash("OCF2B"), bool>, ccOCF2B>;
using BitsOCF2A = setl::BitsRW<setl::SemanticType<setl::hash("OCF2A"), bool>, ccOCF2A>;
using BitsTOV2 = setl::BitsRW<setl::SemanticType<setl::hash("TOV2"), bool>, ccTOV2>;
using FieldsTIFR2 = setl::BitFields<BitsOCF2B, BitsOCF2A, BitsTOV2>;

// Asynchronous Status Register for timer 2.
using BitsEXCLK = setl::BitsRW<setl::SemanticType<setl::hash("EXCLK"), bool>, ccEXCLK>;
using BitsAS2 = setl::BitsRW<setl::SemanticType<setl::hash("AS2"), bool>, ccAS2>;
using BitsTCN2UB = setl::BitsRO<setl::SemanticType<setl::hash("TCN2UB"), bool>, ccTCN2UB>;
using BitsOCR2AUB = setl::BitsRO<setl::SemanticType<setl::hash("OCR2AUB"), bool>, ccOCR2AUB>;
using BitsOCR2BUB = setl::BitsRO<setl::SemanticType<setl::hash("OCR2BUB"), bool>, ccOCR2BUB>;
using BitsTCR2AUB = setl::BitsRO<setl::SemanticType<setl::hash("TCR2AUB"), bool>, ccTCR2AUB>;
using BitsTCR2BUB = setl::BitsRO<setl::SemanticType<setl::hash("TCR2BUB"), bool>, ccTCR2BUB>;
using FieldsASSR = setl::BitFields<
    BitsEXCLK, BitsAS2, BitsTCN2UB, BitsOCR2AUB, BitsOCR2BUB, BitsTCR2AUB, BitsTCR2BUB>;

// General Timer/Counter Control Register for timers 0 & 1.
using BitsTSM = setl::BitsRW<setl::SemanticType<setl::hash("TSM"), bool>, ccTSM>;
using BitsPSRASY = setl::BitsRW<setl::SemanticType<setl::hash("PSRASY"), bool>, ccPSRASY>;
using BitsPSRSYNC = setl::BitsRW<setl::SemanticType<setl::hash("PSRSYNC"), bool>, ccPSRSYNC>;
using FieldsGTCCR = setl::BitFields<BitsTSM, BitsPSRASY, BitsPSRSYNC>;

// Define registers for timer/counter 1
using BitsWGM1_10 = setl::BitsRW<EnumWGM1, NA, NA, ccWGM11, ccWGM10>;
using BitsWGM1_32 = setl::BitsRW<EnumWGM1, ccWGM13, ccWGM12, NA, NA>;
// Define bitfields when treating TCCR1A + TCCR1B as a 16 bit value.
using BitsWGM1_3210 = setl::BitsRW<EnumWGM1, ccWGM13 + 8, ccWGM12 + 8, ccWGM11, ccWGM10>;

using BitsCS11 = setl::BitsRW<EnumCS1, ccCS12, ccCS11, ccCS10>;
// Define bitfields when treating TCCR1A and TCCR1B as a 16 bit value.
using BitsCS11_16 = setl::BitsRW<EnumCS1, ccCS12 + 8, ccCS11 + 8, ccCS10 + 8>;

using BitsICNC1 = setl::BitsRW<bool, ccICNC1>;
using BitsICES1 = setl::BitsRW<bool, ccICES1>;
// Define bitfields when treating TCCR1A and TCCR1B as a 16 bit value.
using BitsICNC1_16 = setl::BitsRW<bool, ccICNC1 + 8>;
using BitsICES1_16 = setl::BitsRW<bool, ccICES1 + 8>;

using FieldsTCCR1A = setl::BitFields<BitsCOM1A8, BitsCOM1B8, BitsWGM1_10>;

using FieldsTCCR1AB = setl::BitFields<
  BitsCOM1A, BitsCOM1B, BitsWGM1_3210, BitsCS11_16, BitsICNC1_16, BitsICES1_16>;

using FieldsTCCR1B = setl::BitFields<BitsWGM1_32, BitsCS11, BitsICES1, BitsICNC1>;

// Add a TCCR1AB 16 bit register using TCCR1A's address.
using rrTCCR1AB = rrTCCR1A::ForType<std::uint16_t>;

// Forced output compare bit definitions for timer/counter 1.
using BitsFOC1A = setl::BitsRW<setl::SemanticType<setl::hash("FOC1A"), bool>, ccFOC1A>;
using BitsFOC1B = setl::BitsRW<setl::SemanticType<setl::hash("FOC1B"), bool>, ccFOC1B>;
using FieldsTCCR1C = setl::BitFields<BitsFOC1A, BitsFOC1B>;


// Timer/Counter1 Interrupt Mask Register.
using BitsOCIE1B = setl::BitsRW<setl::SemanticType<setl::hash("OCIE1B"), bool>, ccOCIE1B>;
using BitsOCIE1A = setl::BitsRW<setl::SemanticType<setl::hash("OCIE1A"), bool>, ccOCIE1A>;
using BitsTOIE1 = setl::BitsRW<setl::SemanticType<setl::hash("TOIE1"), bool>, ccTOIE1>;
using BitsICIE1 = setl::BitsRW<setl::SemanticType<setl::hash("ICIE1"), bool>, ccICIE1>;
using FieldsTIMSK1 = setl::BitFields<BitsICIE1, BitsOCIE1B, BitsOCIE1A, BitsTOIE1>;

// Timer/Counter1 Interrupt Flag Register.
using BitsOCF1B = setl::BitsRW<setl::SemanticType<setl::hash("OCF1B"), bool>, ccOCF1B>;
using BitsOCF1A = setl::BitsRW<setl::SemanticType<setl::hash("OCF1A"), bool>, ccOCF1A>;
using BitsTOV1 = setl::BitsRW<setl::SemanticType<setl::hash("TOV1"), bool>, ccTOV1>;
using BitsICF1 = setl::BitsRW<setl::SemanticType<setl::hash("ICF1"), bool>, ccICF1>;
using FieldsTIFR1 = setl::BitFields<BitsICF1, BitsOCF1B, BitsOCF1A, BitsTOV1>;

using BitsTCNT1 = setl::BitsRW<setl::SemanticType<setl::hash("TCNT1"), std::uint16_t>,
  ccTCNT1H7 + 8,
  ccTCNT1H6 + 8,
  ccTCNT1H5 + 8,
  ccTCNT1H4 + 8,
  ccTCNT1H3 + 8,
  ccTCNT1H2 + 8,
  ccTCNT1H1 + 8,
  ccTCNT1H0 + 8,
  ccTCNT1L7,
  ccTCNT1L6,
  ccTCNT1L5,
  ccTCNT1L4,
  ccTCNT1L3,
  ccTCNT1L2,
  ccTCNT1L1,
  ccTCNT1L0
  >;
using FieldsTCNT1 = setl::BitFields<BitsTCNT1>;

using BitsOCR1A = setl::BitsRW<setl::SemanticType<setl::hash("OCR1A"), std::uint16_t>,
  ccOCR1AH7 + 8,
  ccOCR1AH6 + 8,
  ccOCR1AH5 + 8,
  ccOCR1AH4 + 8,
  ccOCR1AH3 + 8,
  ccOCR1AH2 + 8,
  ccOCR1AH1 + 8,
  ccOCR1AH0 + 8,
  ccOCR1AL7,
  ccOCR1AL6,
  ccOCR1AL5,
  ccOCR1AL4,
  ccOCR1AL3,
  ccOCR1AL2,
  ccOCR1AL1,
  ccOCR1AL0
>;
using FieldsOCR1A = setl::BitFields<BitsOCR1A>;

using BitsOCR1B = setl::BitsRW<setl::SemanticType<setl::hash("OCR1B"), std::uint16_t>,
  ccOCR1BH7 + 8,
  ccOCR1BH6 + 8,
  ccOCR1BH5 + 8,
  ccOCR1BH4 + 8,
  ccOCR1BH3 + 8,
  ccOCR1BH2 + 8,
  ccOCR1BH1 + 8,
  ccOCR1BH0 + 8,
  ccOCR1BL7,
  ccOCR1BL6,
  ccOCR1BL5,
  ccOCR1BL4,
  ccOCR1BL3,
  ccOCR1BL2,
  ccOCR1BL1,
  ccOCR1BL0
>;
using FieldsOCR1B = setl::BitFields<BitsOCR1B>;

// Defines the 16 bits for the ICR1 register.
using BitsICR1 = setl::BitsRW<setl::SemanticType<setl::hash("ICR1"), std::uint16_t>,
  ccICR1H7 + 8,
  ccICR1H6 + 8,
  ccICR1H5 + 8,
  ccICR1H4 + 8,
  ccICR1H3 + 8,
  ccICR1H2 + 8,
  ccICR1H1 + 8,
  ccICR1H0 + 8,
  ccICR1L7,
  ccICR1L6,
  ccICR1L5,
  ccICR1L4,
  ccICR1L3,
  ccICR1L2,
  ccICR1L1,
  ccICR1L0
>;

using FieldsICR1 = setl::BitFields<BitsICR1>;

} // namespace ardo::sys::avr::arch_atmega328p
