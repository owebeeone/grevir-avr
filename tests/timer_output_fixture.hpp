#pragma once
#include "timer_configuration_fixture.hpp"
#include <grevir/avr/gpio.hpp>
// Legacy enum is fixture data, not a production device inventory.
namespace ardo::sys::avr::base {
enum class EnumCOMn : std::uint8_t { disconnect, toggle, clear, set };
}
#include <grevir/avr/timer/output.hpp>

namespace timer_output_fixture {
using namespace ardo::sys::avr::base;
struct NoBarrier {};
template <typename Access, typename Counter = std::uint16_t>
struct Model : timer_configuration_fixture::Model<Access, Counter> {
  using Base = timer_configuration_fixture::Model<Access, Counter>;
  using CompareABits = typename Base::CompareBits;
  using CompareBBits = setl::BitsRW<setl::SemanticType<204, Counter>>;
  using ComABits = setl::BitsRW<EnumCOMn, 1, 0>;
  using ComBBits = setl::BitsRW<EnumCOMn, 3, 2>;
  using PinABits = setl::BitsRW<bool, 0>;
  using PinBBits = setl::BitsRW<bool, 1>;
  using PortReg = typename Base::template Reg<16, std::uint8_t, PinABits, PinBBits>;
  using DdReg = typename Base::template Reg<17, std::uint8_t, PinABits, PinBBits>;
  using PinReg = typename Base::template Reg<18, std::uint8_t, PinABits, PinBBits>;
  template <typename Bit>
  using GpioDef = GpioPortDefinition<Bit, Bit, Bit, PortReg, PinReg, DdReg>;
  struct PinA : GpioPort<PinA, GpioDef<PinABits>, NoBarrier> {};
  struct PinB : GpioPort<PinB, GpioDef<PinBBits>, NoBarrier> {};
  using CompareA = OutputCompare<CompareABits, void, void, ComABits, ComABits, PinA>;
  using CompareB = OutputCompare<CompareBBits, void, void, ComBBits, ComBBits, PinB>;
  using Registers = decltype(std::tuple_cat(std::declval<typename Base::Registers>(),
    std::declval<std::tuple<typename Base::template Reg<8, std::uint8_t, ComABits, ComBBits>,
      typename Base::template Reg<10, Counter, CompareBBits>>>()));
  using Definition = TimerDefinition<typename Base::WaveBits, typename Base::ClockBits,
    void, void, typename Base::CountBits, std::tuple<CompareA, CompareB>,
    TimerCapture<typename Base::CaptureBits, void, typename Base::EdgeBits>, Registers>;
  using Config = TimerConfiguration<Definition, 1000, 16000000, timer_configuration_fixture::CaptureSettings>;
};
using SettingsA = TimerOutputPinSettings<OcrEnum::OcrA, true>;
using SettingsB = TimerOutputPinSettings<OcrEnum::OcrB, false>;
} // namespace timer_output_fixture
