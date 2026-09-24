#pragma once

#include <grevir/base/compat/cstdint.hpp>
#include <grevir/base/compat/limits.hpp>
#include <grevir/base/compat/type_traits.hpp>

namespace ardo::sys::avr::base {

template <typename Enum> struct TccrEnumTraits;
inline constexpr std::uint32_t InvalidClockDivider = (std::numeric_limits<std::uint32_t>::max)();

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
template <typename T>
constexpr bool positiveFinite(T value) {
  return value > 0 && value <= (std::numeric_limits<T>::max)();
}

// Positive divisors only. Unlike (value + divisor - 1) / divisor, cannot overflow.
constexpr std::uint32_t ceilDivide(std::uint32_t value, std::uint32_t divisor) {
  return value / divisor + (value % divisor != 0);
}

// A floating conversion of an integer maximum can round UP to the first
// unrepresentable integer. Use the exact exclusive power-of-two bound then.
template <typename R, typename F>
constexpr bool fitsNonnegativeInteger(F value) {
  static_assert(std::is_integral_v<R> && std::is_floating_point_v<F>);
  if constexpr (std::numeric_limits<F>::digits >= std::numeric_limits<R>::digits) {
    return value >= 0 && value <= static_cast<F>((std::numeric_limits<R>::max)());
  } else {
    const F upper = static_cast<F>((std::numeric_limits<R>::max)() / 2 + 1) * F{2};
    return value >= 0 && value < upper;
  }
}

template <typename F>
constexpr std::uint32_t timerCount(F count) {
  if (!(count >= 1 && fitsNonnegativeInteger<std::uint32_t>(count))) {
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
  static_assert(std::is_arithmetic_v<T>);
  if (!nfp::positiveFinite(minimum_frequency) || timer_clock_frequency == 0
      || resolution_bits_of_top_comparator == 0 || resolution_bits_of_top_comparator > 32) {
    return InvalidClockDivider;
  }
  const std::uint32_t phase = phase_correct_mode ? 2 : 1;
  const auto capacity = (std::numeric_limits<std::uint32_t>::max)()
    >> (32 - resolution_bits_of_top_comparator);
  if constexpr (std::is_integral_v<T>) {
    if (minimum_frequency > timer_clock_frequency / phase) {
      return InvalidClockDivider;
    }
    // ceil(ceil(a / b) / c) == ceil(a / (b*c)) for positive integers.
    // Keep the odd clock tick in phase-correct mode until the final rounding.
    const auto ticks = nfp::ceilDivide(timer_clock_frequency, phase);
    const auto counts = nfp::ceilDivide(ticks, static_cast<std::uint32_t>(minimum_frequency));
    return nfp::ceilDivide(counts, capacity);
  } else {
    const T clock = static_cast<T>(timer_clock_frequency) / static_cast<T>(phase);
    if (minimum_frequency > clock) {
      return InvalidClockDivider;
    }
    const T needed = clock / static_cast<T>(capacity) / minimum_frequency;
    if (!(needed < static_cast<T>(InvalidClockDivider))) {
      return InvalidClockDivider;
    }
    const auto whole = static_cast<std::uint32_t>(needed);
    return whole + (static_cast<T>(whole) < needed ? 1u : 0u);
  }
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
  static_assert(std::is_arithmetic_v<T>);
  const auto divider = Traits::FreqMapping::findDividerMultiple(clock_divider_enum);
  if (!nfp::positiveFinite(top_count) || timer_clock_frequency == 0
      || divider == 0 || divider == InvalidClockDivider) {
    return R{0};
  }
  const std::uint32_t phase = phase_correct_mode ? 2 : 1;
  if constexpr (std::is_integral_v<T> && std::is_integral_v<R>) {
    if (top_count > timer_clock_frequency) {
      return R{0}; // The frequency is below one, including for wide input types.
    }
    const auto count = static_cast<std::uint32_t>(top_count);
    const auto phase_clock = timer_clock_frequency / phase;
    const auto count_clock = phase_clock / count;
    const auto frequency = count_clock / divider;
    if constexpr (std::numeric_limits<R>::digits < 32) {
      constexpr auto maximum = static_cast<std::uint32_t>((std::numeric_limits<R>::max)());
      // Preserve rejection of max + a fraction, rather than truncating it to max.
      if (frequency > maximum || (frequency == maximum
          && (timer_clock_frequency % phase != 0 || phase_clock % count != 0
            || count_clock % divider != 0))) {
        return R{0};
      }
    }
    return static_cast<R>(frequency);
  } else {
    // Floating arithmetic is selected by the caller's input or result type.
    using F = std::common_type_t<T, R>;
    const F frequency = static_cast<F>(timer_clock_frequency)
      / static_cast<F>(phase) / static_cast<F>(top_count) / static_cast<F>(divider);
    if constexpr (std::is_integral_v<R>) {
      if (!nfp::fitsNonnegativeInteger<R>(frequency)) {
        return R{0};
      }
    } else {
      if (!(frequency <= (std::numeric_limits<R>::max)())) {
        return R{0};
      }
    }
    return static_cast<R>(frequency);
  }
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
  static_assert(std::is_arithmetic_v<T>);
  if (!nfp::positiveFinite(selected_frequency) || timer_clock_frequency == 0) {
    return 0;
  }
  const auto mapped = Traits::FreqMapping::findDividerMultiple(clock_divider);
  const auto divider = mapped == InvalidClockDivider || mapped == 0 ? 1u : mapped;
  const std::uint32_t phase = phase_correct_mode ? 2 : 1;
  if constexpr (std::is_integral_v<T>) {
    if (selected_frequency > timer_clock_frequency) {
      return 0;
    }
    return timer_clock_frequency / phase / divider
      / static_cast<std::uint32_t>(selected_frequency);
  } else {
    return nfp::timerCount(static_cast<T>(timer_clock_frequency)
      / static_cast<T>(phase) / static_cast<T>(divider) / selected_frequency);
  }
}

} // namespace ardo::sys::avr::base
