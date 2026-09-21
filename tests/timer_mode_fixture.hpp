#pragma once

#include <grevir/avr/timer/mode.hpp>
#include <grevir/base/int_scaler.hpp>

// Legacy ATmega328P mode data, used only as host fixtures. This is not a device backend.
namespace ardo::sys::avr::base {

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
    TimerTop::built_in, setl::mersenne(8)>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::pwm_phase_correct_8bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, setl::mersenne(8)>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::ctc_ocra, TimerMode::ctc, TimerPwmMode::none,
    TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM0, EnumWGM0::fast_pwm_8bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, setl::mersenne(8)>,
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
    TimerTop::built_in, setl::mersenne(16)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_8bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, setl::mersenne(8)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_9bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, setl::mersenne(9)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::pwm_phase_correct_10bit, TimerMode::pwm, TimerPwmMode::phase_correct,
    TimerTop::built_in, setl::mersenne(10)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::ctc_ocra, TimerMode::ctc, TimerPwmMode::none, TimerTop::ocra>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::ctc_icr, TimerMode::ctc, TimerPwmMode::none, TimerTop::icr>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_8bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, setl::mersenne(8)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_9bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, setl::mersenne(9)>,
  WaveformGeneratorMode<
    EnumWGM1, EnumWGM1::fast_pwm_10bit, TimerMode::pwm, TimerPwmMode::fast,
    TimerTop::built_in, setl::mersenne(10)>,
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

// Check that built_in_type works.
static_assert(Wgm1TopCountMapping::built_in_type<
  TimerMode::pwm, TimerPwmMode::fast, setl::mersenne(10)>::wgm_value == EnumWGM1::fast_pwm_10bit,
  "Should evaluate to EnumWGM1::fast_pwm_10bit");

// Check that getter works.
static_assert(
  WaveformGeneratorModeGet<
    WaveformGeneratorModeParam::wgm_value,
    WaveformGeneratorMode<
      EnumWGM1, EnumWGM1::fast_pwm_icr, TimerMode::pwm, TimerPwmMode::fast,
      TimerTop::icr>
    > == EnumWGM1::fast_pwm_icr, "Should evaluate to EnumWGM1::fast_pwm_icr");

static_assert(
  WaveformGeneratorModeGet<
    WaveformGeneratorModeParam::timer_mode,
    WaveformGeneratorMode<
      EnumWGM1, EnumWGM1::fast_pwm_icr, TimerMode::pwm, TimerPwmMode::fast,
      TimerTop::icr>
    > == TimerMode::pwm, "Should evaluate to TimerMode::pwm");

static_assert(
  WaveformGeneratorModeGet<
    WaveformGeneratorModeParam::timer_pwm_mode,
    WaveformGeneratorMode<
      EnumWGM1, EnumWGM1::fast_pwm_icr, TimerMode::pwm, TimerPwmMode::fast,
      TimerTop::icr>
    > == TimerPwmMode::fast, "Should evaluate to TimerPwmMode::fast");

static_assert(
  WaveformGeneratorModeGet<
    WaveformGeneratorModeParam::timer_top,
    WaveformGeneratorMode<
      EnumWGM1, EnumWGM1::fast_pwm_icr, TimerMode::pwm, TimerPwmMode::fast,
      TimerTop::icr>
    > == TimerTop::icr, "Should evaluate to TimerTop::icr");

static_assert(
  WaveformGeneratorModeGet<
    WaveformGeneratorModeParam::built_in_top,
    WaveformGeneratorMode<
      EnumWGM1, EnumWGM1::fast_pwm_10bit, TimerMode::pwm, TimerPwmMode::fast,
      TimerTop::built_in, setl::mersenne(10)>
    > == setl::mersenne(10), "Should evaluate to setl::mersenne(10)");


template <>
struct WgmEnumTraits<EnumWGM1> {
  using Modes = Wgm1TopCountMapping;
};

} // namespace ardo::sys::avr::base
