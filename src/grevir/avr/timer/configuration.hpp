#pragma once

#include <grevir/avr/timer/pwm_clock.hpp>
#include <grevir/avr/timer/definition.hpp>
#include <grevir/base/int_scaler.hpp>

namespace ardo::sys::avr::base {

template <typename Settings, typename Config> struct TimerOutputPin;
namespace nfp {
template <typename T> struct SingleTimerMode {
  static_assert(std::tuple_size_v<T> == 1, "GREVIR_TIMER_CONFIGURATION_MODE_NOT_UNIQUE");
};
template <typename T> struct SingleTimerMode<std::tuple<T>> { using type = T; };
template <typename... Shifts> struct TimerCountMask {
  static constexpr std::uint32_t value = (std::uint32_t{0} | ...
    | setl::bit_shift(Shifts::mask, Shifts::shift));
};

// Counter and TOP values are unsigned scalars of at most 32 logical bits.
// Derive capacity from the field mapping, not the storage register's width.
template <typename Bits> struct TimerCountField {
  static_assert(std::is_unsigned_v<typename Bits::type> && sizeof(typename Bits::type) <= 4,
    "GREVIR_TIMER_INVALID_COUNT_TYPE");
  using Mapping = typename Bits::template ShiftMaskInfo<std::uint32_t>::group
    ::template apply<TimerCountMask>;
  static constexpr std::uint32_t capacity = Mapping::value;
  static_assert(capacity <= std::numeric_limits<typename Bits::type>::max()
      && capacity != 0 && (capacity & (capacity + std::uint32_t{1})) == 0,
    "GREVIR_TIMER_INVALID_COUNT_FIELD");
  static constexpr std::uint8_t width = [] {
    std::uint8_t result = 0;
    for (auto mask = capacity; mask != 0; mask >>= 1) {
      ++result;
    }
    return result;
  }();
};

template <typename TimerDef, typename ClockTraits, typename T>
T configuredTimerFrequency(setl::Optional<std::uint32_t> top, std::uint32_t clock) {
  using BitsCS = typename TimerDef::BitsCS;
  using Modes = typename TimerDef::ModeTraits::Modes;
  BitsCS cs;
  typename TimerDef::BitsWGM_16 wgm;
  TimerDef::Registers::Read(cs, wgm);
  const auto pwm = Modes::template getParamFor<WaveformGeneratorModeParam::timer_pwm_mode>(wgm.value);
  if (!pwm.is_present()) {
    return static_cast<T>(-1); // Preserve the legacy unavailable-mode sentinel.
  }
  if (!top.is_present()) {
    const auto source = Modes::template getParamFor<WaveformGeneratorModeParam::timer_top>(wgm.value);
    if (!source.is_present()) {
      return static_cast<T>(-1);
    }
    top = TimerDef::get_timer_top(source.get(), wgm.value);
    if (!top.is_present()) {
      return static_cast<T>(-1);
    }
  }
  if (pwm.get() == TimerPwmMode::none) {
    return getTimerFrequency<T, typename BitsCS::type, ClockTraits>(top.get(), cs.value, clock, false);
  }
  return getPwmFrequency<T, typename BitsCS::type, ClockTraits>(
    top.get(), cs.value, clock, TimerPwmModePhaseCorrect(pwm.get()));
}
} // namespace nfp

/**
 * Configure the timer using accurate frequency mode. This uses a programmable
 * top value (as opposed to a built in top value of 0xff, 0x1ff or 0x3ff).
 * On 16 bit timers, the "ICR" (Input Capture Register) may be used as a top value
 * comparator otherwise all the timers will also allow the "OCRA" (Output Compare
 * Register) which clashes with one of the PWM (A) outputs reducing to one PWM
 * output for the timer using the OCRB register.
 *
 * The timers are selected so that the most accurate setup_frequency is provided
 * however for vey high frequencies, fewer bits of resolution are provided hence
 * reducing both the frequency accuracy and the PWM resolution.
 */
template <
  typename w_TimerDef,
  std::uint32_t w_setup_frequency,
  std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
  TimerPwmMode w_timer_pwm_mode,
  TimerTop w_timer_top,
  typename w_ClockTraits = TccrEnumTraits<typename w_TimerDef::BitsCS::type>
>
struct TimerPwmConfigutation {
  using TimerDef = w_TimerDef;
  using ClockTraits = w_ClockTraits;
  using Registers = typename TimerDef::Registers;
  using BitsWGM_16 = typename TimerDef::BitsWGM_16;
  using EnumWGM = typename BitsWGM_16::type;
  using BitsCS = typename TimerDef::BitsCS;
  using BitsTCNT = typename TimerDef::BitsTCNT;
  using EnumCS = typename BitsCS::type;
  static constexpr TimerMode timer_mode = TimerMode::pwm;
  static constexpr TimerPwmMode timer_pwm_mode = w_timer_pwm_mode;
  static constexpr TimerTop timer_top = w_timer_top;
  using Modes = typename TimerDef::ModeTraits::Modes::
      template type<timer_mode, timer_pwm_mode, timer_top>;
  static_assert(!std::is_same_v<std::tuple<>, Modes>,
    "Requested timer_mode and timer_top modes for waveform generator not found.");
  using Mode = typename nfp::SingleTimerMode<Modes>::type;  // There must be only a single mode.

  static constexpr std::uint32_t setup_frequency = w_setup_frequency;
  static constexpr std::uint32_t base_frequency = w_base_frequency;
  static constexpr bool phase_correct_mode = TimerPwmModePhaseCorrect(w_timer_pwm_mode);
  using TopField = nfp::TimerCountField<typename TimerDef::template TimerDefTopRegister<timer_top>>;
  using CounterField = nfp::TimerCountField<BitsTCNT>;
  static constexpr std::uint8_t top_resolution = TopField::width < CounterField::width
    ? TopField::width : CounterField::width;
  static constexpr std::uint32_t capacity = TopField::capacity < CounterField::capacity
    ? TopField::capacity : CounterField::capacity;
  static constexpr EnumCS cs_value = getPwmClockDivider<EnumCS, ClockTraits>(
    setup_frequency, base_frequency, top_resolution, phase_correct_mode);
  static constexpr auto top_count = getPwmTop<EnumCS, ClockTraits>(
    cs_value, setup_frequency, base_frequency, phase_correct_mode);

  static_assert(cs_value != ClockTraits::null_value,
    "Impossible frequency settings for timer.");
  static constexpr std::uint32_t minimum_top = phase_correct_mode ? 2u : 3u;
  static_assert(top_count >= minimum_top && top_count <= capacity,
    "GREVIR_TIMER_CONFIGURATION_COUNT_OUT_OF_RANGE");

  using TimerSetupApplier = setl::ApplierValues<
    setl::ApplierValue<BitsCS, cs_value>,
    setl::ApplierValue<BitsWGM_16, Mode::wgm_value>>;

  /**
   * Set the base frequency for the timer dynamically. Returns the top_count
   * (maximum value) for the compare registers. Invalid requests return zero
   * without register IO. Applying valid settings is not an atomic update.
   */
  template <typename T>
  static std::uint32_t setFrequency(T frequency) {
    EnumCS cs_value = getPwmClockDivider<EnumCS, ClockTraits>(
        frequency, base_frequency, top_resolution, phase_correct_mode);
    if (cs_value == ClockTraits::null_value) {
      return 0;
    }
    auto top_count = getPwmTop<EnumCS, ClockTraits>(
        cs_value, frequency, base_frequency, phase_correct_mode);

    if (top_count < minimum_top || top_count > capacity) {
      return 0;
    }
    Registers::ReadModifyWrite(
      BitsCS{cs_value},
      BitsWGM_16{Mode::wgm_value});
    return top_count;
  }

  /**
   * Get the current frequency of the timer.
   * If no parameters are provided, the current timer settings are used to
   * compute the frequency.
   */
  template <typename T>
  static T getFrequency(
      setl::Optional<std::uint32_t> p_top_count = {},
      std::uint32_t p_base_frequency = base_frequency) {
    return nfp::configuredTimerFrequency<TimerDef, ClockTraits, T>(
      p_top_count, p_base_frequency);
  }
};

/**
 * Configure the timer using a built-in top counter value. 8 bit timers are limited
 * to one top value (0xff) while the 16 bit timers have 3 top values (0xff, 0x1ff and
 * 0x3ff or 8, 9 and 10 bits of resolution.
 *
 * Divider selection uses the waveform period (TOP+1 or 2*TOP). The resulting
 * frequency is no greater than max_frequency; fractional results require an
 * explicitly selected floating-point getFrequency result type.
 */
template <
  typename w_TimerDef,
  std::uint32_t w_max_frequency,
  std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
  TimerPwmMode w_timer_pwm_mode,
  std::uint32_t w_bits_resolution, // Number of bits for the top value.
  typename w_ClockTraits = TccrEnumTraits<typename w_TimerDef::BitsCS::type>
>
struct TimerPwmBuiltinTopConfigutation {
  using TimerDef = w_TimerDef;
  using ClockTraits = w_ClockTraits;
  using Registers = typename TimerDef::Registers;
  using BitsWGM_16 = typename TimerDef::BitsWGM_16;
  using EnumWGM = typename BitsWGM_16::type;
  using BitsCS = typename TimerDef::BitsCS;
  using BitsTCNT = typename TimerDef::BitsTCNT;
  using EnumCS = typename BitsCS::type;

  using TopCountType = typename TimerDef::template OcrType<OcrEnum::OcrA>::OCR::type;

  static constexpr std::uint32_t setup_frequency = w_max_frequency;
  static constexpr std::uint32_t base_frequency = w_base_frequency;
  static constexpr TimerTop timer_top = TimerTop::built_in;
  static constexpr TimerPwmMode timer_pwm_mode = w_timer_pwm_mode;
  static constexpr bool phase_correct_mode = TimerPwmModePhaseCorrect(w_timer_pwm_mode);
  static constexpr std::uint32_t top_resolution = w_bits_resolution;
  static constexpr EnumCS cs_value = getPwmClockDivider<EnumCS, ClockTraits>(
    setup_frequency, base_frequency, top_resolution, phase_correct_mode);
  static_assert(top_resolution > 0 && top_resolution <= nfp::TimerCountField<BitsTCNT>::width
      && top_resolution <= nfp::TimerCountField<typename TimerDef::template OcrType<OcrEnum::OcrA>::OCR>::width,
    "GREVIR_TIMER_CONFIGURATION_INVALID_RESOLUTION");
  static_assert(cs_value != ClockTraits::null_value, "GREVIR_TIMER_CONFIGURATION_INVALID_FREQUENCY");
  static constexpr std::uint32_t actual_divider = findDividerMultiple<EnumCS, ClockTraits>(cs_value);
  static constexpr auto top_count = setl::mersenne(top_resolution);
  // Find the WGM bits for these settings.
  using Mode = typename TimerDef::ModeTraits::Modes::
    template built_in_type<TimerMode::pwm, timer_pwm_mode, top_count>;
  static_assert(!std::is_void_v<Mode>, "GREVIR_TIMER_CONFIGURATION_BUILTIN_MODE_NOT_FOUND");
  static constexpr EnumWGM wgm_value = Mode::wgm_value;

  // Timer setup for "built in" top setting.
  using TimerSetupApplier = setl::ApplierValues<
    setl::ApplierValue<BitsCS, cs_value>,
    setl::ApplierValue<BitsWGM_16, wgm_value>>;


  using PwmEnableA = setl::ApplierValues<
    setl::ApplierValue<BitsWGM_16, wgm_value>>;

  /**
 * Set the base frequency for the timer dynamically. Returns the top_count
 * (maximum value) for the compare registers. Invalid requests return zero
 * without register IO. Applying valid settings is not an atomic update.
 */
  template <typename T>
  static std::uint32_t setFrequency(T frequency) {
    EnumCS cs_value = getPwmClockDivider<EnumCS, ClockTraits>(
      frequency, base_frequency, top_resolution, phase_correct_mode);

    if (cs_value == ClockTraits::null_value) {
      return 0;
    }
    Registers::ReadModifyWrite(
      BitsCS{ cs_value },
      BitsWGM_16{ wgm_value });

    return top_count;
  }

  /**
   * Get the current frequency of the timer.
   * If no parameters are provided, the current timer settings are used to
   * compute the frequency.
   */
  template <typename T>
  static T getFrequency(
    setl::Optional<std::uint32_t> p_top_count = {},
    std::uint32_t p_base_frequency = base_frequency) {
    return nfp::configuredTimerFrequency<TimerDef, ClockTraits, T>(
      p_top_count, p_base_frequency);
  }

};

template <
  TimerMode w_timer_mode,
  TimerPwmMode w_timer_pwm_mode,
  TimerTop w_timer_top>
struct TimerSettings {
  static constexpr TimerMode timer_mode = w_timer_mode;
  static constexpr TimerPwmMode timer_pwm_mode = w_timer_pwm_mode;
  static constexpr TimerTop timer_top = w_timer_top;
};

template <
  typename w_TimerDef,
  std::uint32_t w_setup_frequency,
  std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
  typename w_TimerSettings,
  typename w_ClockTraits = TccrEnumTraits<typename w_TimerDef::BitsCS::type>
>
struct TimerConfiguration;

/**
 * Constant frequency using ICR top register.
 */
template <
  typename w_TimerDef,
  std::uint32_t w_setup_frequency,
  std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
  TimerPwmMode w_timer_pwm_mode,
  TimerTop w_timer_top,
  typename w_ClockTraits
>
struct TimerConfiguration<
    w_TimerDef,
    w_setup_frequency,
    w_base_frequency,
    TimerSettings<
        TimerMode::pwm,
        w_timer_pwm_mode,
        w_timer_top>, w_ClockTraits> {

  using TimerDef = w_TimerDef;
  using ClockTraits = w_ClockTraits;
  using TopCountBits = typename TimerDef::template TimerDefTopRegister<w_timer_top>;
  using TopCountType = typename TopCountBits::type;

  using Config = TimerPwmConfigutation<
    w_TimerDef, w_setup_frequency, w_base_frequency, w_timer_pwm_mode, w_timer_top, ClockTraits>;
  using Registers = typename Config::Registers;

  using TopApplier = setl::ApplierValues<
      setl::ApplierValue<TopCountBits, Config::top_count>>;

  /**
   * Setup the timer for these settings.
   */
  static void setupTimer() {
    Config::TimerSetupApplier::template apply<Registers>();
    TopApplier::template apply<Registers>();
  }

  template <typename T>
  static std::uint32_t setFrequency(T frequency) {
    const auto top_count = Config::setFrequency(frequency);
    if (top_count == 0) {
      return 0;
    }
    Registers::ReadModifyWrite(TopCountBits{static_cast<TopCountType>(top_count)});
    return top_count;
  }

  /**
   * Get the current top count.
   */
  static typename TopCountBits::type get_top_count() {
    TopCountBits top_count_bits;
    Registers::Read(top_count_bits);
    return top_count_bits.value;
  }

  /**
   * Get the current frequency of the timer.
   */
  template <typename T>
  static T getFrequency() {
    return Config::template getFrequency<T>();
  }

  template <typename w_TimerOutputPinSettings>
  using OutputPin = TimerOutputPin<w_TimerOutputPinSettings, TimerConfiguration>;

};

/**
 * Timer settings for built in top values.
 * The 8 bit timers have only an 8 bit (0xff) top value but
 * the 16 bt timers have 8, 9 and 10 bit built in top values.
 */


template <
  TimerMode w_timer_mode,
  TimerPwmMode w_timer_pwm_mode,
  std::uint8_t w_bits_resolution=8u
>
struct TimerBuiltInSettings {
  static constexpr TimerMode timer_mode = w_timer_mode;
  static constexpr TimerPwmMode timer_pwm_mode = w_timer_pwm_mode;
  static constexpr TimerTop timer_top = TimerTop::built_in;
  static constexpr std::uint8_t bits_resolution = w_bits_resolution;
};

template <
  typename w_TimerDef,
  std::uint32_t w_setup_frequency,
  std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
  TimerPwmMode w_timer_pwm_mode,
  std::uint8_t w_bits_resolution,
  typename w_ClockTraits
>
struct TimerConfiguration<
  w_TimerDef,
  w_setup_frequency,
  w_base_frequency,
  TimerBuiltInSettings<
    TimerMode::pwm,
    w_timer_pwm_mode,
    w_bits_resolution>, w_ClockTraits> {

  using TimerDef = w_TimerDef;
  using ClockTraits = w_ClockTraits;

  using Config = TimerPwmBuiltinTopConfigutation<
    w_TimerDef, w_setup_frequency, w_base_frequency, w_timer_pwm_mode, w_bits_resolution, ClockTraits>;
  using Registers = typename Config::Registers;
  using TopCountType = typename Config::TopCountType;

  /**
   * Setup the timer for these settings.
   */
  static void setupTimer() {
    Config::TimerSetupApplier::template apply<Registers>();
  }


  template <typename T>
  static std::uint32_t setFrequency(T frequency) {
    const auto top_count = Config::setFrequency(frequency);

    // Top count it determined by w_bits_resolution and is set by the WGM bits.
    return top_count;
  }

  /**
   * Get the current frequency of the timer.
   */
  template <typename T>
  static T getFrequency() {
    return Config::template getFrequency<T>();
  }

  /**
   * Get the current top count.
   */
  static TopCountType get_top_count() {
    return Config::top_count;
  }

  /**
   * A PWM timer pin configuration.
   */
  template <typename w_TimerOutputPinSettings>
  using OutputPin = TimerOutputPin<w_TimerOutputPinSettings, TimerConfiguration>;

};


} // namespace ardo::sys::avr::base
