#pragma once
#include <grevir/avr/timer/clock.hpp>

namespace ardo::sys::avr::base {

// Hardware PWM encoding: fast PWM counts 0..TOP inclusive; dual-slope PWM
// counts up and down with a period of 2*TOP. Integer paths need only 32 bits.
template <typename Enum, typename Traits = TccrEnumTraits<Enum>, typename T>
constexpr Enum getPwmClockDivider(T frequency, std::uint32_t clock,
    std::uint8_t bits, bool phase_correct) {
  if (phase_correct) {
    return getClockDivider<Enum, Traits>(frequency, clock, bits, true);
  }
  if (!nfp::positiveFinite(frequency) || clock == 0 || bits == 0 || bits > 32) {
    return Traits::null_value;
  }
  std::uint32_t needed = 0;
  if constexpr (std::is_integral_v<T>) {
    if (frequency > clock) { return Traits::null_value; }
    const auto ticks = nfp::ceilDivide(clock, static_cast<std::uint32_t>(frequency));
    needed = bits == 32 ? 1 : nfp::ceilDivide(ticks, std::uint32_t{1} << bits);
  } else {
    if (frequency > static_cast<T>(clock)) { return Traits::null_value; }
    const T cycles = bits == 32 ? static_cast<T>(std::uint32_t{1} << 31) * T{2}
      : static_cast<T>(std::uint32_t{1} << bits);
    const T required = static_cast<T>(clock) / cycles / frequency;
    if (!(required < static_cast<T>(InvalidClockDivider))) { return Traits::null_value; }
    const auto whole = static_cast<std::uint32_t>(required);
    needed = whole + (static_cast<T>(whole) < required ? 1u : 0u);
  }
  return Traits::FreqMapping::findLargestLessThanOrEq(needed, Traits::null_value);
}

template <typename Enum, typename Traits = TccrEnumTraits<Enum>, typename T>
constexpr std::uint32_t getPwmTop(Enum cs, T frequency, std::uint32_t clock, bool phase_correct) {
  if (findDividerMultiple<Enum, Traits>(cs) == InvalidClockDivider) { return 0; }
  const auto count = getClockTimerTop<Enum, Traits>(cs, frequency, clock, phase_correct);
  return phase_correct || count == 0 ? count : count - std::uint32_t{1};
}

template <typename R, typename Enum, typename Traits = TccrEnumTraits<Enum>>
constexpr R getPwmFrequency(std::uint32_t top, Enum cs, std::uint32_t clock, bool phase_correct) {
  if (phase_correct) { return getTimerFrequency<R, Enum, Traits>(top, cs, clock, true); }
  if (top == (std::numeric_limits<std::uint32_t>::max)()) {
    if constexpr (std::is_integral_v<R>) {
      return R{0}; // The complete 2^32-tick period exceeds a uint32 clock.
    } else {
      const R cycles = static_cast<R>(std::uint32_t{1} << 31) * R{2};
      return getTimerFrequency<R, Enum, Traits>(cycles, cs, clock, false);
    }
  }
  return getTimerFrequency<R, Enum, Traits>(top + std::uint32_t{1}, cs, clock, false);
}

} // namespace ardo::sys::avr::base
