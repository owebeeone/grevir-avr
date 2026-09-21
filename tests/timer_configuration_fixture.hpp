#pragma once

#include <grevir/avr/timer/configuration.hpp>
#include <grevir/avr/register.hpp>
#include <grevir/avr/register_definitions.hpp>
#include "timer_clock_fixture.hpp"
#include "timer_mode_fixture.hpp"

namespace timer_configuration_fixture {
using namespace ardo::sys::avr::base;
template <typename Access, typename Counter = std::uint16_t, typename Top = Counter>
struct Model {
  using WaveBits = setl::BitsRW<EnumWGM1, 7, 6, 5, 4>;
  using ClockBits = setl::BitsRW<EnumCS1, 2, 1, 0>;
  using CountBits = setl::BitsRW<setl::SemanticType<201, Counter>>;
  using CaptureBits = setl::BitsRW<setl::SemanticType<202, Top>>;
  using CompareBits = setl::BitsRW<setl::SemanticType<203, Counter>>;
  using EdgeBits = setl::BitsRW<bool, 0>;
  using Binding = RegisterSelector<Access>;
  template <std::ptrdiff_t Address, typename T, typename... Bits>
  using Reg = Register<setl::BitFields<Bits...>, ardo::sys::avr::nfp::MemRegisterDef<T, Address>,
    Binding::template IoAccessor>;
  using Registers = std::tuple<Reg<0, std::uint8_t, ClockBits, WaveBits>, Reg<2, Counter, CountBits>,
    Reg<4, Top, CaptureBits>, Reg<6, Counter, CompareBits>>;
  using Compare = OutputCompare<CompareBits, void, void, void, void, void>;
  using Definition = TimerDefinition<WaveBits, ClockBits, void, void, CountBits,
    std::tuple<Compare>, TimerCapture<CaptureBits, void, EdgeBits>, Registers>;
};
using CaptureSettings = TimerSettings<TimerMode::pwm, TimerPwmMode::fast, TimerTop::icr>;
using CompareSettings = TimerSettings<TimerMode::pwm, TimerPwmMode::phase_correct, TimerTop::ocra>;
using FixedSettings = TimerBuiltInSettings<TimerMode::pwm, TimerPwmMode::fast, 8>;
} // namespace timer_configuration_fixture
