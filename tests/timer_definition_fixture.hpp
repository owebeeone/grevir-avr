#pragma once

#include <grevir/avr/timer/definition.hpp>
#include "gpio_fixture.hpp"
#include "timer_mode_fixture.hpp"

namespace timer_definition_mock {
using namespace ardo::sys::avr::base;
using Memory = avr_mock::Memory;
using Fixture = avr_mock::Fixture;

template <unsigned Id, typename T>
using ValueBits = setl::BitsRW<setl::SemanticType<Id, T>>;
using WaveBits = setl::BitsRW<EnumWGM1, 3, 2, 1, 0>;
using CountBits = ValueBits<100, std::uint16_t>;
using CaptureBits = ValueBits<101, std::uint16_t>;
using CompareABits = ValueBits<102, std::uint16_t>;
using CompareBBits = ValueBits<103, std::uint16_t>;
using NoiseBits = setl::BitsRW<setl::SemanticType<104, bool>, 7>;
using EdgeBits = setl::BitsRW<setl::SemanticType<105, bool>, 6>;

template <std::ptrdiff_t Address, typename T, typename... Bits>
using Reg = Register<setl::BitFields<Bits...>,
  ardo::sys::avr::nfp::MemRegisterDef<T, Address>, avr_mock::Binding::IoAccessor>;
using Registers = std::tuple<Reg<16, std::uint8_t, WaveBits>,
  Reg<17, std::uint8_t, NoiseBits, EdgeBits>, Reg<18, std::uint16_t, CountBits>,
  Reg<20, std::uint16_t, CaptureBits>, Reg<22, std::uint16_t, CompareABits>,
  Reg<24, std::uint16_t, CompareBBits>>;

// No output/interrupt configuration is needed for these metadata/TOP tests.
using CompareA = OutputCompare<CompareABits, void, void, void, void, void>;
using CompareB = OutputCompare<CompareBBits, void, void, void, void, void>;
using Capture = TimerCapture<CaptureBits, NoiseBits, EdgeBits>;
using NoCapture = TimerCapture<void, void, void>;
using Definition = TimerDefinition<WaveBits, void, void, void, CountBits,
  std::tuple<CompareA, CompareB>, Capture, Registers>;
using NoCaptureDefinition = TimerDefinition<WaveBits, void, void, void, CountBits,
  std::tuple<CompareA, CompareB>, NoCapture, Registers>;
using CompareOnlyDefinition = TimerDefinition<WaveBits, void, void, void, CountBits,
  std::tuple<CompareA, CompareB>, NoCapture, Registers,
  setl::ValueTuple<TimerTop, TimerTop::ocra>>;
using NoTopDefinition = TimerDefinition<WaveBits, void, void, void, CountBits,
  std::tuple<CompareA, CompareB>, NoCapture, Registers, setl::ValueTuple<TimerTop>>;
} // namespace timer_definition_mock
