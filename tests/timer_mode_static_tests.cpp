#include "timer_mode_fixture.hpp"

namespace ardo::sys::avr::base {
using FastBuiltIn = Wgm1TopCountMapping::type<TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in>;
static_assert(std::tuple_size_v<FastBuiltIn> == 3);
static_assert(std::tuple_element_t<0, FastBuiltIn>::built_in_top == 255);
static_assert(std::tuple_element_t<1, FastBuiltIn>::built_in_top == 511);
static_assert(std::tuple_element_t<2, FastBuiltIn>::built_in_top == 1023);
static_assert(Wgm1TopCountMapping::found<TimerMode::ctc, TimerPwmMode::none, TimerTop::icr>);
static_assert(!Wgm0TopCountMapping::found<TimerMode::ctc, TimerPwmMode::none, TimerTop::icr>);
static_assert(!Wgm1TopCountMapping::found<TimerMode::ctc, TimerPwmMode::fast, TimerTop::ocra>);
static_assert(std::is_same_v<Wgm1TopCountMapping::type<
  TimerMode::normal, TimerPwmMode::fast, TimerTop::built_in>, std::tuple<>>);
static_assert(std::is_void_v<Wgm0TopCountMapping::built_in_type<TimerMode::pwm, TimerPwmMode::fast, 511>>);
static_assert(std::is_void_v<Wgm1TopCountMapping::built_in_type<TimerMode::pwm, TimerPwmMode::fast, 256>>);
using Empty = WaveformGeneratorModes<>;
static_assert(!Empty::found<TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in>);
static_assert(std::is_same_v<Empty::type<TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in>, std::tuple<>>);
static_assert(std::is_void_v<Empty::built_in_type<TimerMode::pwm, TimerPwmMode::fast, 255>>);

// Selection depends on metadata, not numeric encoding or table ordering.
enum class SyntheticCode : std::uint8_t { small = 90, large = 2, capture = 41 };
using Large = WaveformGeneratorMode<SyntheticCode, SyntheticCode::large,
  TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 1023>;
using Small = WaveformGeneratorMode<SyntheticCode, SyntheticCode::small,
  TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 255>;
using Capture = WaveformGeneratorMode<SyntheticCode, SyntheticCode::capture,
  TimerMode::pwm, TimerPwmMode::phase_freq_correct, TimerTop::icr>;
using Synthetic = WaveformGeneratorModes<Large, Capture, Small>;
static_assert(std::is_same_v<Synthetic::built_in_type<TimerMode::pwm, TimerPwmMode::fast, 255>, Small>);
static_assert(std::is_same_v<Synthetic::type<TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in>,
  std::tuple<Large, Small>>);
static_assert(Capture::built_in_top == UnspecifiedTimerTop);
static_assert(sizeof(Capture::built_in_top) == 4);
static_assert(!TimerPwmModePhaseCorrect(TimerPwmMode::none));
static_assert(!TimerPwmModePhaseCorrect(TimerPwmMode::fast));
static_assert(TimerPwmModePhaseCorrect(TimerPwmMode::phase_correct));
static_assert(TimerPwmModePhaseCorrect(TimerPwmMode::phase_freq_correct));

// The out-parameter overload supports constant evaluation for plain enum values.
constexpr TimerTop captureTop() {
  TimerTop result = TimerTop::none;
  Synthetic::getParamFor<WaveformGeneratorModeParam::timer_top>(SyntheticCode::capture, result);
  return result;
}
static_assert(captureTop() == TimerTop::icr);
} // namespace ardo::sys::avr::base
