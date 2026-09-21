#pragma once

#include <grevir/avr/timer/clock.hpp>
#include <grevir/avr/timer/mode.hpp>

namespace ardo::sys::avr::arch_atmega328p {
using base::DividerMapping;
using base::DividerMappings;
using base::TimerMode;
using base::TimerPwmMode;
using base::TimerTop;
using base::WaveformGeneratorMode;
using base::WaveformGeneratorModes;
template <typename T> struct TccrEnumTraits;
enum class EnumCS1 : unsigned char {
  no_clk = 0b000,
  clk1 = 0b001,
  clk8 = 0b010,
  clk64 = 0b011,
  clk256 = 0b100,
  clk1024 = 0b101,
  ext_clk_falling = 0b110,
  ext_clk_rising = 0b111,
};

// Timer counter 0 uses the same bit definitions.
using EnumCS0 = EnumCS1;

/**
 * Mapping of EnumCS1 to divider value.
 */
// Note: must be in ascending divider order.
using Cs1DividerMapping = DividerMappings<
  DividerMapping<EnumCS1, EnumCS1::clk1, 1>,
  DividerMapping<EnumCS1, EnumCS1::clk8, 8>,
  DividerMapping<EnumCS1, EnumCS1::clk64, 64>,
  DividerMapping<EnumCS1, EnumCS1::clk256, 256>,
  DividerMapping<EnumCS1, EnumCS1::clk1024, 1024>
>;

/**
 * Traits for EnumCS1 used by getClockDivider to discover
 * the clock divider for a specific frequency.
 */
template <>
struct TccrEnumTraits<EnumCS1> {
  using EnumT = EnumCS1;
  using FreqMapping = Cs1DividerMapping;
  static constexpr EnumT null_value = EnumT::no_clk;
  static constexpr EnumT start_range = EnumT::clk1;
  static constexpr EnumT end_range = EnumT::clk1024;
};


/**
 * Define register TCCR2B CS2n definitions.
 */
enum class EnumCS2 : unsigned char {
  no_clk = 0b000,
  clk1 = 0b001,
  clk8 = 0b010,
  clk32 = 0b011,
  clk64 = 0b100,
  clk128 = 0b101,
  clk256 = 0b110,
  clk1024 = 0b111
};

/**
 * Mapping of EnumCS2 to divider value.
 */
 // Note: must be in ascending divider order.
using Cs2DividerMapping = DividerMappings<
  DividerMapping<EnumCS2, EnumCS2::clk1, 1>,
  DividerMapping<EnumCS2, EnumCS2::clk8, 8>,
  DividerMapping<EnumCS2, EnumCS2::clk32, 32>,
  DividerMapping<EnumCS2, EnumCS2::clk64, 64>,
  DividerMapping<EnumCS2, EnumCS2::clk128, 128>,
  DividerMapping<EnumCS2, EnumCS2::clk256, 256>,
  DividerMapping<EnumCS2, EnumCS2::clk1024, 1024>
>;

/**
 * Traits for EnumCS2 used by getClockDivider to discover
 * the clock divider for a specific frequency.
 */
template <>
struct TccrEnumTraits<EnumCS2> {
  using EnumT = EnumCS2;
  using FreqMapping = Cs2DividerMapping;
  static constexpr EnumT null_value = EnumT::no_clk;
  static constexpr EnumT start_range = EnumT::clk1;
  static constexpr EnumT end_range = EnumT::clk1024;
};

enum class EnumWGM0 : unsigned char {
  normal = 0b000,
  pwm_phase_correct_8bit = 0b001,
  ctc_ocra = 0b010,
  fast_pwm_8bit = 0b011,
  reserved_8 = 0b100,
  pwm_phase_correct_ocra = 0b101,
  reserved_6 = 0b110,
  fast_pwm_ocra = 0b111,
};

// Timer/counter 2 uses the same WGM bits as timer/counter 0.
using EnumWGM2 = EnumWGM0;

template <typename WgmEnum>
struct WgmEnumTraits;

/**
 * Mapping of EnumWGM0 to properties.
 */
using Wgm0TopCountMapping = WaveformGeneratorModes<
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::normal, TimerMode::normal, TimerPwmMode::none,
    TimerTop::built_in, 255>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::pwm_phase_correct_8bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, 255>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::ctc_ocra, TimerMode::ctc, TimerPwmMode::none,
    TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::fast_pwm_8bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, 255>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::pwm_phase_correct_ocra, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::fast_pwm_ocra, TimerMode::pwm, TimerPwmMode::fast, TimerTop::ocra>
>;

template <>
struct WgmEnumTraits<EnumWGM0> {
  using Modes = Wgm0TopCountMapping;
};


/**
 * Defines for operating modes of timer/counter 1.
 */
enum class EnumWGM1 : unsigned char {
  normal = 0b0000,
  pwm_phase_correct_8bit = 0b0001,
  pwm_phase_correct_9bit = 0b0010,
  pwm_phase_correct_10bit = 0b0011,
  ctc_ocra = 0b0100,
  fast_pwm_8bit = 0b0101,
  fast_pwm_9bit = 0b0110,
  fast_pwm_10bit = 0b0111,
  pwm_phase_freq_correct_icr = 0b1000,
  pwm_phase_freq_correct_ocra = 0b1001,
  pwm_phase_correct_icr = 0b1010,
  pwm_phase_correct_ocra = 0b1011,
  ctc_icr = 0b1100,
  reserved_d = 0b1101,
  fast_pwm_icr = 0b1110,
  fast_pwm_ocra = 0b1111,
};

/**
 * Mapping of EnumWGM1 to properties.
 */
using Wgm1TopCountMapping = WaveformGeneratorModes<
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::normal, TimerMode::normal, TimerPwmMode::none,
    TimerTop::built_in, 65535>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_8bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, 255>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_9bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, 511>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_10bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, 1023>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::ctc_ocra, TimerMode::ctc, TimerPwmMode::none, TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::ctc_icr, TimerMode::ctc, TimerPwmMode::none, TimerTop::icr>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_8bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, 255>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_9bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, 511>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_10bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, 1023>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_freq_correct_icr, TimerMode::pwm, TimerPwmMode::phase_freq_correct,
    TimerTop::icr>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_freq_correct_ocra, TimerMode::pwm, TimerPwmMode::phase_freq_correct,
    TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_icr, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::icr>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_ocra, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_icr, TimerMode::pwm, TimerPwmMode::fast, TimerTop::icr>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_ocra, TimerMode::pwm, TimerPwmMode::fast, TimerTop::ocra>
>;














template <>
struct WgmEnumTraits<EnumWGM1> {
  using Modes = Wgm1TopCountMapping;
};

} // namespace ardo::sys::avr::arch_atmega328p
