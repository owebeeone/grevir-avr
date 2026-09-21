#pragma once

#include <grevir/base/meta/tuple_types.hpp>
#include <grevir/base/optional.hpp>
#include <grevir/base/compat/cstdint.hpp>
#include <grevir/base/compat/limits.hpp>
#include <grevir/base/compat/tuple.hpp>
#include <grevir/base/compat/type_traits.hpp>

namespace ardo::sys::avr::base {

// Metadata marker for a TOP supplied by a register, rather than a built-in value.
inline constexpr std::uint32_t UnspecifiedTimerTop = std::numeric_limits<std::uint32_t>::max();

/**
 * Indicates the type of waveform generator mode being used.
 */
enum class TimerMode : unsigned {
  normal,
  ctc,
  pwm
};

/**
 * Indicates the type of PWM mode being used.
 */
enum class TimerPwmMode : unsigned {
  none,
  fast,
  phase_correct,
  phase_freq_correct
};

constexpr bool TimerPwmModePhaseCorrect(TimerPwmMode timer_pwm_mode) {
  return timer_pwm_mode == TimerPwmMode::phase_correct
    || timer_pwm_mode == TimerPwmMode::phase_freq_correct;
}

/**
 * The timer top count used.
 */
enum class TimerTop : unsigned {
  none,
  built_in,
  ocra,
  icr
};

/**
 * Provides enum metadata for Waveform Generator Mode enums.
 */
template <typename w_EnumT,
          w_EnumT w_wgm_value,
          TimerMode w_timer_mode,
          TimerPwmMode w_timer_pwm_mode,
          TimerTop w_timer_top,
          std::uint32_t w_built_in_top=UnspecifiedTimerTop>
struct WaveformGeneratorMode {
  using EnumT = w_EnumT;
  static constexpr EnumT wgm_value = w_wgm_value;
  static constexpr TimerMode timer_mode = w_timer_mode;
  static constexpr TimerPwmMode timer_pwm_mode = w_timer_pwm_mode;
  static constexpr TimerTop timer_top = w_timer_top;
  static constexpr std::uint32_t built_in_top = w_built_in_top;
};

/**
 * Provides enum metadata for Waveform Generator Mode parameters.
 */
enum class WaveformGeneratorModeParam : unsigned {
  wgm_value,
  timer_mode,
  timer_pwm_mode,
  timer_top,
  built_in_top
};

/**
 * Provides getter templates for WaveformGeneratorMode enum metadata.
 */
struct WaveformGeneratorModeGetters {
  template <
          WaveformGeneratorModeParam w_param,
          typename T>
  struct Getter {};

  /** Getter for the WGMx value for the mode provided. */
  template <
        typename w_EnumT,
        w_EnumT w_wgm_value,
        TimerMode w_timer_mode,
        TimerPwmMode w_timer_pwm_mode,
        TimerTop w_timer_top,
        std::uint32_t w_built_in_divider>
  struct Getter<
    WaveformGeneratorModeParam::wgm_value,
    WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>> {
    static constexpr w_EnumT value = w_wgm_value;
  };

  /** Getter for the timer mode for the mode provided. */
  template <
        typename w_EnumT,
        w_EnumT w_wgm_value,
        TimerMode w_timer_mode,
        TimerPwmMode w_timer_pwm_mode,
        TimerTop w_timer_top,
        std::uint32_t w_built_in_divider>
  struct Getter<
    WaveformGeneratorModeParam::timer_mode,
    WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>> {
    static constexpr TimerMode value = w_timer_mode;
  };

  /** Getter for the timer PWM mode for the mode provided. */
  template <
        typename w_EnumT,
        w_EnumT w_wgm_value,
        TimerMode w_timer_mode,
        TimerPwmMode w_timer_pwm_mode,
        TimerTop w_timer_top,
        std::uint32_t w_built_in_divider>
  struct Getter<
    WaveformGeneratorModeParam::timer_pwm_mode,
    WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>> {
    static constexpr TimerPwmMode value = w_timer_pwm_mode;
  };

  /** Getter for the timer top for the mode provided. */
  template <
        typename w_EnumT,
        w_EnumT w_wgm_value,
        TimerMode w_timer_mode,
        TimerPwmMode w_timer_pwm_mode,
        TimerTop w_timer_top,
        std::uint32_t w_built_in_divider>
  struct Getter<
    WaveformGeneratorModeParam::timer_top,
    WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>> {
    static constexpr TimerTop value = w_timer_top;
  };

  /** Getter for the timer top for the mode provided. */
  template <
    typename w_EnumT,
    w_EnumT w_wgm_value,
    TimerMode w_timer_mode,
    TimerPwmMode w_timer_pwm_mode,
    TimerTop w_timer_top,
    std::uint32_t w_built_in_divider>
  struct Getter<
    WaveformGeneratorModeParam::built_in_top,
    WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>> {
    static constexpr std::uint32_t value = w_built_in_divider;
  };
};

template <
      WaveformGeneratorModeParam w_param,
      typename T>
constexpr auto WaveformGeneratorModeGet = WaveformGeneratorModeGetters::Getter<w_param, T>::value;

/**
 * Getter for WaveformGeneratorMode metadata.
 */
// template <
//       WaveformGeneratorModeParam w_param,
//       typename w_EnumT,
//       w_EnumT w_wgm_value,
//       TimerMode w_timer_mode,
//       TimerPwmMode w_timer_pwm_mode,
//       TimerTop w_timer_top,
//       std::uint32_t w_built_in_divider>
// constexpr auto WaveformGeneratorModeGet =
//     WaveformGeneratorModeGetters::Getter<
//       w_param,
//       WaveformGeneratorMode<w_EnumT, w_wgm_value, w_timer_mode, w_timer_pwm_mode, w_timer_top, w_built_in_divider>
//     >::value;

/**
 * Waveform generator modes container. Provides searh facilities to select
 * the most appropriate waveform generator modes.
 */
template <typename...Ts>
struct WaveformGeneratorModes;

template <typename T>
struct WaveformGeneratorModesFinder;

template <>
struct WaveformGeneratorModesFinder<std::tuple<>> {
  template <std::uint32_t w_built_in_divider>
  using type = void;
};

template <typename T, typename...Ts>
struct WaveformGeneratorModesFinder<std::tuple<T, Ts...>> {
  using Rest = WaveformGeneratorModesFinder<std::tuple<Ts...>>;
  template <std::uint32_t w_built_in_top>
  using type = std::conditional_t<
      T::built_in_top == w_built_in_top, T,
      typename Rest::template type<w_built_in_top>>;
};

// Specialization for the empty list.
template <>
struct WaveformGeneratorModes<> {
  template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, TimerTop timer_top>
  static constexpr bool found = false;

  template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, TimerTop timer_top>
  using type = std::tuple<>;

  template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, std::uint32_t built_in_top>
  using built_in_type = void;

  template <WaveformGeneratorModeParam w_param, typename EnumT, typename Result>
  static constexpr void getParamFor(EnumT, Result&) {
    // A miss leaves the caller's result unchanged, including a preexisting value.
  }
};

template <typename T, typename...Ts>
struct WaveformGeneratorModes<T, Ts...> {
  using Rest = WaveformGeneratorModes<Ts...>;
  using Mode = T;
 private:
   template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, TimerTop timer_top>
   struct Helper {
     static constexpr bool this_selected =
       Mode::timer_mode == timer_mode
       && Mode::timer_pwm_mode == timer_pwm_mode
       && Mode::timer_top == timer_top;
     using type = std::conditional_t<this_selected, std::tuple<Mode>, std::tuple<>>;
   };
 public:

  template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, TimerTop timer_top>
  using type = setl::tuple_concat_t<
    typename Helper<timer_mode, timer_pwm_mode, timer_top>::type,
    typename Rest::template type<timer_mode, timer_pwm_mode, timer_top>>;

  template <TimerMode timer_mode, TimerPwmMode timer_pwm_mode, TimerTop timer_top>
  static constexpr bool found = std::tuple_size_v<type<timer_mode, timer_pwm_mode, timer_top>> != 0;


  template <TimerMode timer_mode,
            TimerPwmMode timer_pwm_mode,
            std::uint32_t w_built_in_divider>
  using built_in_type = typename WaveformGeneratorModesFinder<
    typename WaveformGeneratorModes::
        template type<timer_mode, timer_pwm_mode, TimerTop::built_in>>::
    template type<w_built_in_divider>;

  template <WaveformGeneratorModeParam w_param, typename EnumT, typename Result>
  static constexpr void getParamFor(EnumT wgm_value, Result& result) {
    if (wgm_value == Mode::wgm_value) {
      result = WaveformGeneratorModeGetters::Getter<w_param, Mode>::value;
    } else {
      Rest::template getParamFor<w_param>(wgm_value, result);
    }
  }

  template <WaveformGeneratorModeParam w_param>
  using ParamType = std::remove_cv_t<decltype(
      WaveformGeneratorModeGetters::Getter<w_param, Mode>::value)>;

  template <WaveformGeneratorModeParam w_param>
  using ParamReturnType = setl::Optional<ParamType<w_param>>;

  /**
   * Returns the value of the parameter w_param for the mode associated with wgm_value.
   * This will work at run time.
   */
  template <WaveformGeneratorModeParam w_param, typename EnumT>
  static constexpr ParamReturnType<w_param> getParamFor(EnumT wgm_value) {
    ParamReturnType<w_param> result;
    getParamFor<w_param>(wgm_value, result);
    return result;
  }

};

} // namespace ardo::sys::avr::base
