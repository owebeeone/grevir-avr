#pragma once

#include <grevir/avr/timer/clock.hpp>

// Legacy clock encodings retained only as fixture data.
namespace ardo::sys::avr::base {
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

} // namespace ardo::sys::avr::base
