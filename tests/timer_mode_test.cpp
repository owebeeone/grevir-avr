#include "timer_mode_fixture.hpp"
#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
#include <array>

namespace {
using namespace ardo::sys::avr::base;
using Param = WaveformGeneratorModeParam;
struct Expected {
  unsigned code;
  TimerMode mode;
  TimerPwmMode pwm;
  TimerTop top;
  std::uint32_t count;
};
template <typename Modes, typename Code, std::size_t N>
void checkModes(const std::array<Expected, N>& expected) {
  for (const auto& item : expected) {
    const auto code = static_cast<Code>(item.code);
    const auto mode = Modes::template getParamFor<Param::timer_mode>(code);
    const auto pwm = Modes::template getParamFor<Param::timer_pwm_mode>(code);
    const auto top = Modes::template getParamFor<Param::timer_top>(code);
    const auto count = Modes::template getParamFor<Param::built_in_top>(code);
    const auto returned_code = Modes::template getParamFor<Param::wgm_value>(code);
    REQUIRE(mode.is_present());
    REQUIRE(pwm.is_present());
    REQUIRE(top.is_present());
    REQUIRE(count.is_present());
    REQUIRE(returned_code.is_present());
    CHECK(mode.get() == item.mode);
    CHECK(pwm.get() == item.pwm);
    CHECK(top.get() == item.top);
    CHECK(count.get() == item.count);
    CHECK(returned_code.get() == code);
  }
}
}

TEST_CASE("eight bit waveform fixtures retain legacy mode metadata", "[avr]") {
  const std::array<Expected, 6> expected{{
    {0, TimerMode::normal, TimerPwmMode::none, TimerTop::built_in, 255},
    {1, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::built_in, 255},
    {2, TimerMode::ctc, TimerPwmMode::none, TimerTop::ocra, UnspecifiedTimerTop},
    {3, TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 255},
    {5, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::ocra, UnspecifiedTimerTop},
    {7, TimerMode::pwm, TimerPwmMode::fast, TimerTop::ocra, UnspecifiedTimerTop}
  }};
  checkModes<Wgm0TopCountMapping, EnumWGM0>(expected);
}

TEST_CASE("sixteen bit waveform fixtures retain legacy mode metadata", "[avr]") {
  const std::array<Expected, 15> expected{{
    {0, TimerMode::normal, TimerPwmMode::none, TimerTop::built_in, 65535},
    {1, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::built_in, 255},
    {2, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::built_in, 511},
    {3, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::built_in, 1023},
    {4, TimerMode::ctc, TimerPwmMode::none, TimerTop::ocra, UnspecifiedTimerTop},
    {5, TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 255},
    {6, TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 511},
    {7, TimerMode::pwm, TimerPwmMode::fast, TimerTop::built_in, 1023},
    {8, TimerMode::pwm, TimerPwmMode::phase_freq_correct, TimerTop::icr, UnspecifiedTimerTop},
    {9, TimerMode::pwm, TimerPwmMode::phase_freq_correct, TimerTop::ocra, UnspecifiedTimerTop},
    {10, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::icr, UnspecifiedTimerTop},
    {11, TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::ocra, UnspecifiedTimerTop},
    {12, TimerMode::ctc, TimerPwmMode::none, TimerTop::icr, UnspecifiedTimerTop},
    {14, TimerMode::pwm, TimerPwmMode::fast, TimerTop::icr, UnspecifiedTimerTop},
    {15, TimerMode::pwm, TimerPwmMode::fast, TimerTop::ocra, UnspecifiedTimerTop}
  }};
  checkModes<Wgm1TopCountMapping, EnumWGM1>(expected);
}

TEST_CASE("reserved waveform codes return no metadata without overwriting output", "[avr]") {
  for (unsigned code : {4u, 6u, 255u}) {
    CHECK_FALSE(Wgm0TopCountMapping::getParamFor<Param::timer_mode>(static_cast<EnumWGM0>(code)).is_present());
  }
  for (unsigned code : {13u, 255u}) {
    const auto mode = static_cast<EnumWGM1>(code);
    CHECK_FALSE(Wgm1TopCountMapping::getParamFor<Param::timer_mode>(mode).is_present());
    CHECK_FALSE(Wgm1TopCountMapping::getParamFor<Param::timer_pwm_mode>(mode).is_present());
    CHECK_FALSE(Wgm1TopCountMapping::getParamFor<Param::timer_top>(mode).is_present());
    CHECK_FALSE(Wgm1TopCountMapping::getParamFor<Param::built_in_top>(mode).is_present());
    CHECK_FALSE(Wgm1TopCountMapping::getParamFor<Param::wgm_value>(mode).is_present());
    TimerTop existing = TimerTop::icr;
    Wgm1TopCountMapping::getParamFor<Param::timer_top>(mode, existing);
    CHECK(existing == TimerTop::icr);
  }
  setl::Optional<TimerTop> existing(TimerTop::ocra);
  WaveformGeneratorModes<>::getParamFor<Param::timer_top>(EnumWGM1::normal, existing);
  REQUIRE(existing.is_present());
  CHECK(existing.get() == TimerTop::ocra);
}

TEST_CASE_METHOD(avr_mock::Fixture, "selected waveform encodings apply to split mock fields", "[avr]") {
  using LowBits = setl::BitsRW<EnumWGM1, 1, 0>;
  using HighBits = setl::BitsRW<EnumWGM1, 4, 3, setl::NA, setl::NA>;
  using LowDef = ardo::sys::avr::nfp::MemRegisterDef<std::uint8_t, 1>;
  using HighDef = ardo::sys::avr::nfp::MemRegisterDef<std::uint8_t, 2>;
  using LowReg = Register<setl::BitFields<LowBits>, LowDef, avr_mock::Binding::IoAccessor>;
  using HighReg = Register<setl::BitFields<HighBits>, HighDef, avr_mock::Binding::IoAccessor>;
  using Registers = setl::RegisterSelector<std::tuple<LowReg, HighReg>>;
  using Selected = std::tuple_element_t<0,
    Wgm1TopCountMapping::type<TimerMode::pwm, TimerPwmMode::fast, TimerTop::icr>>;
  avr_mock::Memory::bytes[1] = 0xa1;
  avr_mock::Memory::bytes[2] = 0x45;
  using Values = setl::ApplierValues<setl::ApplierValue<LowBits, Selected::wgm_value>,
    setl::ApplierValue<HighBits, Selected::wgm_value>>;
  Values::apply<Registers>();
  CHECK(avr_mock::Memory::bytes[1] == 0xa2);
  CHECK(avr_mock::Memory::bytes[2] == 0x5d);
  LowBits low;
  HighBits high;
  Registers::Read(low, high);
  const auto code = static_cast<EnumWGM1>(static_cast<unsigned>(low.value) | static_cast<unsigned>(high.value));
  const auto top = Wgm1TopCountMapping::getParamFor<Param::timer_top>(code);
  REQUIRE(top.is_present());
  CHECK(top.get() == TimerTop::icr);
}
