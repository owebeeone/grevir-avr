#include "timer_clock_fixture.hpp"

namespace ardo::sys::avr::base {
static_assert(getClockDividerMultiple(1, 511, 8, true) == 2);
static_assert(getClockDividerMultiple(1, 0xffffffffu, 32, false) == 1);
static_assert(getClockTimerTop(EnumCS1::clk1, 1, 0xffffffffu, false) == 0xffffffffu);
static_assert(getTimerFrequency<std::uint32_t>(1, EnumCS1::clk1, 0xffffffffu, false) == 0xffffffffu);
static_assert(getTimerFrequency<std::uint8_t>(2, EnumCS1::clk1, 511, false) == 0);
static_assert(getClockTimerTop(EnumCS1::clk1, 1.0f, 0xffffffffu, false) == 0);

static_assert(
  getClockDivider<EnumCS1>(100000, 16000000, 16, false) == EnumCS1::clk1,
  "Clockdivider computation failed or 100kHz and 16 bit comparator.");

static_assert(
  getClockDivider<EnumCS2>(100000, 16000000, 16, false) == EnumCS2::clk1,
  "Clockdivider computation failed or 100kHz and 16 bit comparator.");

static_assert(
  getClockDivider<EnumCS1>(2, 16000000, 16, true) == EnumCS1::clk64,
  "Clockdivider computation failed or 2Hz and 16 bit comparator in phase correct mode.");

static_assert(
  getClockDivider<EnumCS1>(2, 16000000, 16, false) == EnumCS1::clk256,
  "Clockdivider computation failed or 2Hz and 16 bit comparator.");

static_assert(
  getClockDivider<EnumCS2>(2, 16000000, 16, false) == EnumCS2::clk128,
  "Clockdivider computation failed or 2Hz and 16 bit comparator.");

static_assert(
  getClockDivider<EnumCS1>(0.5, 16000000, 16, false) == EnumCS1::clk1024,
  "Clockdivider computation failed or 0.5Hz and 16 bit comparator.");

static_assert(
  getClockTimerTop(EnumCS1::clk1, 100000, 16000000, true) == 80ul,
  "getClockTimerTop computation failed or 100kHz and clk1 clock divider on phase correct mode.");

static_assert(
  getClockTimerTop(EnumCS1::clk1024, 0.5, 16000000, true) == 15625ul,
  "getClockTimerTop computation failed or 0.5kHz and clk1024 clock divider on phase correct mode.");

static_assert(
  getClockTimerTop(EnumCS1::no_clk, 1, 16000000, true) == 8000000ul,
  "getClockTimerTop computation failed for invalid divider.");

static_assert(
  getTimerFrequency<std::uint32_t>(80ul, EnumCS1::clk1, 16000000, true) == 100000ul,
  "getTimerFrequency computation failed for divider.");

static_assert(
  getTimerFrequency<float>(15625, EnumCS1::clk1024, 16000000, true) == 0.5f,
  "getTimerFrequency computation failed for divider.");

} // namespace ardo::sys::avr::base
