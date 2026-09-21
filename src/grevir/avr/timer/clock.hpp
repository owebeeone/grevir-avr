#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ardo::sys::avr::base {

template <typename Enum> struct TccrEnumTraits;
inline constexpr std::uint32_t InvalidClockDivider = std::numeric_limits<std::uint32_t>::max();

/**
 * Provides divider constants for Clock Selector enums.
 */
template <typename w_EnumT, w_EnumT w_cs_value, std::uint32_t w_divider>
struct DividerMapping {
  using EnumT = w_EnumT;
  static constexpr EnumT cs_value = w_cs_value;
  static constexpr std::uint32_t divider = w_divider;
  static_assert(w_divider > 0 && w_divider < InvalidClockDivider,
    "GREVIR_TIMER_INVALID_DIVIDER");
};

template <typename...Ts>
struct DividerMappings;

template <typename T>
struct DividerMappings<T> {

  using EnumT = typename T::EnumT;


  /**
   * Returns the divider associated with cs_value. If not found, returns InvalidClockDivider.
   */
  static constexpr std::uint32_t findDividerMultiple(EnumT cs_value) {
    return cs_value == T::cs_value ? T::divider : InvalidClockDivider;
  }

  /**
   * Legacy name retained: selects the smallest available divider >= required.
   * Returns the supplied null selector when the request exceeds the table.
   */
  static constexpr EnumT findLargestLessThanOrEq(std::uint32_t divider,
      EnumT null_value = TccrEnumTraits<EnumT>::null_value) {
    return divider <= T::divider ? T::cs_value : null_value;
  }
};

template <typename T, typename...Ts>
struct DividerMappings<T, Ts...> {
  using EnumT = typename T::EnumT;
  static_assert(sizeof(DividerMappings<Ts...>) > 0);
  static_assert((std::is_same_v<EnumT, typename Ts::EnumT> && ...),
    "GREVIR_TIMER_MIXED_SELECTOR_TYPES");
  static_assert(((T::cs_value != Ts::cs_value) && ...),
    "GREVIR_TIMER_DUPLICATE_SELECTOR");
  static_assert(((T::divider < Ts::divider) && ...),
    "GREVIR_TIMER_UNORDERED_DIVIDERS");

  /**
   * Returns the divider associated with cs_value. If not found, returns InvalidClockDivider.
   */
  static constexpr std::uint32_t findDividerMultiple(EnumT cs_value) {
    return cs_value == T::cs_value
        ? T::divider
        : DividerMappings<Ts...>::findDividerMultiple(cs_value);
  }

  /**
   * Legacy name retained: selects the smallest available divider >= required.
   * Returns the supplied null selector when the request exceeds the table.
   */
  static constexpr EnumT findLargestLessThanOrEq(std::uint32_t divider,
      EnumT null_value = TccrEnumTraits<EnumT>::null_value) {
    return divider <= T::divider
        ? T::cs_value
        : DividerMappings<Ts...>::findLargestLessThanOrEq(divider, null_value);
  }
};


namespace nfp {
constexpr bool positiveFinite(long double value) {
  return value > 0 && value <= std::numeric_limits<long double>::max();
}

constexpr std::uint32_t timerCount(long double count) {
  if (!(count >= 1 && count <= std::numeric_limits<std::uint32_t>::max())) {
    return 0;
  }
  return static_cast<std::uint32_t>(count);
}
} // namespace nfp

/** Minimum integer divider needed to fit the legacy count convention.
 * Returns InvalidClockDivider for invalid inputs or an unrepresentable request.
 * Width is explicit (1..32); no host-sized unsigned shifts/products are used.
 */
template <typename T>
constexpr std::uint32_t getClockDividerMultiple(
    T minimum_frequency, std::uint32_t timer_clock_frequency,
    std::uint8_t resolution_bits_of_top_comparator, bool phase_correct_mode) {
  const long double frequency = static_cast<long double>(minimum_frequency);
  if (!nfp::positiveFinite(frequency) || timer_clock_frequency == 0
      || resolution_bits_of_top_comparator == 0 || resolution_bits_of_top_comparator > 32) {
    return InvalidClockDivider;
  }
  const long double clock = static_cast<long double>(timer_clock_frequency)
    / (phase_correct_mode ? 2 : 1);
  if (frequency > clock) {
    return InvalidClockDivider;
  }
  const auto capacity = (std::uint64_t{1} << resolution_bits_of_top_comparator) - 1;
  const long double needed = clock / capacity / frequency;
  if (!(needed < InvalidClockDivider)) {
    return InvalidClockDivider;
  }
  const auto whole = static_cast<std::uint32_t>(needed);
  return whole + (static_cast<long double>(whole) < needed ? 1u : 0u);
}

// Traits can be supplied explicitly or through the legacy enum specialization.
template <typename EnumT, typename Traits = TccrEnumTraits<EnumT>, typename T>
constexpr EnumT getClockDivider(
    T minimum_frequency, std::uint32_t timer_clock_frequency,
    std::uint8_t resolution_bits_of_top_comparator, bool phase_correct_mode) {
  const auto needed = getClockDividerMultiple(minimum_frequency, timer_clock_frequency,
    resolution_bits_of_top_comparator, phase_correct_mode);
  if (needed == InvalidClockDivider) {
    return Traits::null_value;
  }
  return Traits::FreqMapping::findLargestLessThanOrEq(needed, Traits::null_value);
}

/** Frequency under the inherited count model: clock / phase / count / divider.
 * Unknown selectors, invalid inputs and unrepresentable results return zero.
 * Fractional results require a floating-point R.
 */
template <typename R, typename EnumT, typename Traits = TccrEnumTraits<EnumT>, typename T>
constexpr R getTimerFrequency(
    T top_count, EnumT clock_divider_enum, std::uint32_t timer_clock_frequency,
    bool phase_correct_mode) {
  static_assert(std::is_arithmetic_v<R> && !std::is_same_v<R, bool>);
  const auto divider = Traits::FreqMapping::findDividerMultiple(clock_divider_enum);
  const long double count = static_cast<long double>(top_count);
  if (!nfp::positiveFinite(count) || timer_clock_frequency == 0
      || divider == 0 || divider == InvalidClockDivider) {
    return R{0};
  }
  const long double frequency = static_cast<long double>(timer_clock_frequency)
    / (phase_correct_mode ? 2 : 1) / count / divider;
  if (!(frequency <= std::numeric_limits<R>::max())) {
    return R{0};
  }
  return static_cast<R>(frequency);
}

template <typename EnumT, typename Traits = TccrEnumTraits<EnumT>>
constexpr std::uint32_t findDividerMultiple(EnumT value) {
  return Traits::FreqMapping::findDividerMultiple(value);
}

/** Legacy count helper, retaining its documented unit-divider fallback for an
 * unmapped selector. Invalid frequency/clock or out-of-range counts return zero.
 * This is not a waveform-specific hardware TOP encoding; that layer is separate.
 */
template <typename EnumT, typename Traits = TccrEnumTraits<EnumT>, typename T>
constexpr std::uint32_t getClockTimerTop(
    EnumT clock_divider, T selected_frequency, std::uint32_t timer_clock_frequency,
    bool phase_correct_mode) {
  const long double frequency = static_cast<long double>(selected_frequency);
  if (!nfp::positiveFinite(frequency) || timer_clock_frequency == 0) {
    return 0;
  }
  const auto mapped = Traits::FreqMapping::findDividerMultiple(clock_divider);
  const auto divider = mapped == InvalidClockDivider || mapped == 0 ? 1u : mapped;
  return nfp::timerCount(static_cast<long double>(timer_clock_frequency)
    / (phase_correct_mode ? 2 : 1) / divider / frequency);
}

} // namespace ardo::sys::avr::base
