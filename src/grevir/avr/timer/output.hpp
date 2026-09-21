#pragma once

#include <grevir/avr/timer/configuration.hpp>
#include <grevir/base/meta/tuple_algorithms.hpp>

namespace ardo::sys::avr::base {

template <typename w_TimerOutputPinSettings, typename w_TimerConfig>
struct TimerOutputPin {
  using TimerOutputPinSettings = w_TimerOutputPinSettings;
  using TimerConfig = w_TimerConfig;
  using TimerDef = typename TimerConfig::TimerDef;
  using OutputCompare = typename TimerDef::template OcrType<TimerOutputPinSettings::ocrEnum>;


  using COMn = typename OutputCompare::COM8::type;
  using OCR = typename OutputCompare::OCR;

  using GpioPin = typename OutputCompare::GpioDef;
  using CountField = nfp::TimerCountField<OCR>;
  static_assert(CountField::width <= 16, "GREVIR_TIMER_OUTPUT_REQUIRES_8_OR_16_BIT_COUNT");
  static_assert(!(TimerConfig::Config::timer_top == TimerTop::ocra
      && TimerOutputPinSettings::ocrEnum == OcrEnum::OcrA),
    "GREVIR_TIMER_OUTPUT_CONFLICTS_WITH_TOP");
  static constexpr std::uint32_t max_top = [] {
    if constexpr (TimerConfig::Config::timer_top == TimerTop::built_in) {
      return TimerConfig::Config::top_count;
    } else {
      return TimerConfig::Config::capacity;
    }
  }();
  static_assert(max_top <= CountField::capacity, "GREVIR_TIMER_OUTPUT_COUNT_TOO_NARROW");

 private:
  template <typename T>
  static bool validTop(T top) {
    static_assert(std::is_integral_v<T>);
    return top > 0 && top <= CountField::capacity;
  }

  template <typename T>
  static bool finiteValue(T value) {
    static_assert(std::is_arithmetic_v<T>);
    if constexpr (std::is_floating_point_v<T>) {
      return value >= std::numeric_limits<T>::lowest() && value <= std::numeric_limits<T>::max();
    } else {
      return true;
    }
  }

 public:

  // Legacy polarity: true selects clear-on-compare, false set-on-compare.
  constexpr static auto COM_MODE = TimerOutputPinSettings::invert_output
      ? COMn::clear
      : COMn::set;

  static void setupGpio() {
    GpioPin::configure_output();
  }

  static void setupTimerOutputMode() {
    TimerDef::Registers::ReadModifyWrite(typename OutputCompare::COM8{ COM_MODE });
  }

  static void setupGpioOutputMode() {
    TimerDef::Registers::ReadModifyWrite(typename OutputCompare::COM8{ COMn::disconnect });
  }

  static void pwmWriteAbsoluteValue(typename OCR::type value) {
    if (value <= CountField::capacity) {
      TimerDef::Registers::ReadModifyWrite(OCR{ value });
    }
  }

  template <typename T>
  static void pwmWritef(float value, T top_count) {  // 0.0 to 1.0
    if (!validTop(top_count) || !finiteValue(value)) {
      return;
    }
    if (value > 0.0f && value < 1.0f) {
      // Bounds precede conversion; this explicit fractional API uses float.
      pwmWrite(static_cast<typename OCR::type>(value * static_cast<float>(top_count)), top_count);
    } else {
      pwmWrite(value > 0.0f ? top_count : T{0}, top_count);
    }
  }

  /**
   * Write a PWM value to the timer compare register.
   * Forces the output to true or false if the value is outside the range.
   */
  template <typename T, typename U>
  static void pwmWrite(T value, U top_count) {
    if (!validTop(top_count) || !finiteValue(value)) {
      return;
    }
    // Test positivity before comparing signed input against unsigned TOP.
    if (value > 0 && value < top_count) {
      const auto count = static_cast<typename OCR::type>(value);
      if (count != 0) {
        pwmWriteAbsoluteValue(count);
        setupTimerOutputMode();
        return;
      }
    }
    const bool high_end = value > 0 && value >= top_count;
    // Preload the latch before disconnecting timer control of the pad.
    GpioPin::set(high_end == TimerOutputPinSettings::invert_output);
    setupGpioOutputMode();
  }

  /**
   * Adjust the PWM value to the timer compare register for a top count
   * change to maintain a similar duty cycle.
   */
  template <typename T>
  static void pwmAdjust(T prev_top_count, T new_top_count) {
    if (!validTop(prev_top_count) || !validTop(new_top_count) || prev_top_count == new_top_count) {
      // No change.
      return;
    }

    typename OutputCompare::COM8 com_value;
    TimerDef::Registers::Read(com_value);
    if (com_value.value != COM_MODE) {
      // Disconnected or not in this pin configuration's PWM mode.
      return;
    }

    OCR ocr_bits{};
    TimerDef::Registers::Read(ocr_bits);

    // Both factors are at most 65535, so the product fits uint32_t.
    // Widen before multiplying: AVR's 16-bit integer promotion is insufficient.
    const auto bounded = ocr_bits.value < prev_top_count ? ocr_bits.value : prev_top_count;
    const auto new_ocr_value = static_cast<std::uint32_t>(bounded)
      * static_cast<std::uint16_t>(new_top_count) / static_cast<std::uint16_t>(prev_top_count);
    pwmWrite(new_ocr_value, new_top_count);
  }

  /**
   * Initial setup of the timer output pin.
   */
  template <typename T>
  static void setup(T initial_value, T top_count) {
    if (!validTop(top_count) || !finiteValue(initial_value)) {
      return;
    }
    pwmWrite(initial_value, top_count);
    setupGpio();
  }

};

/**
 * Timer settings for output GPIO pins.
 */
template <
  OcrEnum w_ocrEnum,
  bool w_invert_output>
struct TimerOutputPinSettings {
  static constexpr OcrEnum ocrEnum = w_ocrEnum;
  static constexpr bool invert_output = w_invert_output;
};

template <OcrEnum w_ocrEnum>
struct TimerOutputPinSettingsSelector {
  template <typename Settings>
  struct Predicate {
    static constexpr bool value = w_ocrEnum == Settings::ocrEnum;
  };
};


/**
 * A PWM timer configuration containing pin settings.
 * This contains a complete configuration for a timer including the output
 * PWM pins.
 */
template <typename w_Config, typename...w_PinSettings>
struct TimerPwmPinConfiguration {
  using Config = w_Config;

  using TopCountType = typename Config::TopCountType;
  using PinSettings = std::tuple<w_PinSettings...>;
  template <typename w_PinSetting>
  using OutputPin = typename Config::template OutputPin<w_PinSetting>;

  template <int w_pin>
  using PwmPinByArgN = OutputPin<std::tuple_element_t<w_pin, PinSettings>>;

  template <OcrEnum w_ocrEnum>
  using PwmPinByOcrReg = OutputPin<setl::tuple_find_t<
      TimerOutputPinSettingsSelector<w_ocrEnum>::template Predicate, PinSettings>>;

 private:
  template <typename Setting>
  static constexpr bool unique_channel =
    (0 + ... + (Setting::ocrEnum == w_PinSettings::ocrEnum ? 1 : 0)) == 1;
  template <typename Setting>
  static constexpr bool unique_gpio =
    (0 + ... + (std::is_same_v<typename OutputPin<Setting>::GpioPin,
      typename OutputPin<w_PinSettings>::GpioPin> ? 1 : 0)) == 1;
  static_assert((unique_channel<w_PinSettings> && ...), "GREVIR_TIMER_DUPLICATE_OUTPUT_CHANNEL");
  static_assert((unique_gpio<w_PinSettings> && ...), "GREVIR_TIMER_DUPLICATE_OUTPUT_GPIO");

  // Function for setting up a pin.
  template <typename w_PinSetting>
  struct SetupPin {
    using ThisPin = OutputPin<w_PinSetting>;

    static void run(TopCountType set_count, TopCountType top_count) {
      ThisPin::setup(set_count, top_count);
    }
  };

  // Function for adjusting for a new timer top.
  template <typename w_PinSetting>
  struct AdjustPin {
    using ThisPin = OutputPin<w_PinSetting>;

    static void run(TopCountType prev_top_count, TopCountType new_top_count) {
      ThisPin::pwmAdjust(prev_top_count, new_top_count);
    }
  };

 public:
  /**
    * Setup the timer for these settings.
    */
  static void setup() {
    Config::setupTimer();

    // Configure PWM pins.
    auto top_count = Config::get_top_count();
    setl::tuple_for_each<SetupPin, PinSettings>::runall(top_count / 2, top_count);
  }

  /**
    * Set the frequency of the timer.
    */
  template <typename T>
  static std::uint32_t setFrequency(T frequency) {
    if constexpr (sizeof...(w_PinSettings) > 0) {
      auto prev_top_count = Config::get_top_count();
      auto new_top_count = Config::setFrequency(frequency);
      if (new_top_count != 0) {
        setl::tuple_for_each<AdjustPin, PinSettings>::runall(prev_top_count, new_top_count);
      }
      return new_top_count;
    }
    return Config::setFrequency(frequency);
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
  static typename Config::TopCountType get_top_count() {
    return Config::get_top_count();
  }

  /**
    * Write a PWM value to the given pin.
    */
  template <typename w_PinSetting>
  static void pwmWrite(w_PinSetting, std::uint16_t value) {
    static_assert((std::is_same_v<w_PinSetting, w_PinSettings> || ...),
      "GREVIR_TIMER_OUTPUT_NOT_CONFIGURED");
    OutputPin<w_PinSetting>::pwmWrite(value, Config::get_top_count());
  }

};


template <typename w_TimerDef,
  typename w_ClockTraits = TccrEnumTraits<typename w_TimerDef::BitsCS::type>>
struct Timer {

  using TimerDef = w_TimerDef;

  /**
   * The OutputCompare type for the given w_ocr value.
   */
  template <OcrEnum w_ocr>
  using OcrType = typename TimerDef::template OcrType<w_ocr>;

  /**
   * Settings for accurate static frequency timer.
   */
  template <
    std::uint32_t w_setup_frequency,
    std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
    TimerMode w_timer_mode,
    TimerPwmMode w_timer_pwm_mode,
    TimerTop w_timer_top
  >
  using FrequencyAccurate = TimerConfiguration<
      TimerDef,
      w_setup_frequency,
      w_base_frequency,
      TimerSettings<w_timer_mode, w_timer_pwm_mode, w_timer_top>, w_ClockTraits>;

  /**
   * Settings for approximate frequency using built in top values.
   */
  template <
    std::uint32_t w_setup_frequency,
    std::uint32_t w_base_frequency,  // Usually CPU clock frequency.
    TimerMode w_timer_mode,
    TimerPwmMode w_timer_pwm_mode,
    std::uint8_t w_bits_resolution
  >
  using BuiltInTop = TimerConfiguration<
    TimerDef,
    w_setup_frequency,
    w_base_frequency,
    TimerBuiltInSettings<w_timer_mode, w_timer_pwm_mode, w_bits_resolution>, w_ClockTraits>;


  static std::uint32_t getTopCount() {
    using Modes = typename TimerDef::ModeTraits::Modes;
    const auto wgm = TimerDef::Registers::template Read<typename TimerDef::BitsWGM_16>();
    const auto source = Modes::template getParamFor<WaveformGeneratorModeParam::timer_top>(wgm);
    if (!source.is_present()) {
      return 0;
    }
    const auto top = TimerDef::get_timer_top(source.get(), wgm);
    return top.is_present() ? top.get() : 0;
  }

  /**
   * A PWM timer configuration containing pin settings.
   * This contains a complete configuration for a timer including the output
   * PWM pins.
   */
  template <typename w_Config, typename...w_PinSettings>
  using PwmPinConfiguration = TimerPwmPinConfiguration<w_Config, w_PinSettings...>;
};

} // namespace ardo::sys::avr::base
