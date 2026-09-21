#include "timer_configuration_fixture.hpp"
#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
#include <limits>
using namespace timer_configuration_fixture;
using avr_mock::Fixture;
using avr_mock::Memory;
using TimerModel = Model<Memory>;
using Capture = TimerConfiguration<TimerModel::Definition, 1000, 16000000, CaptureSettings>;
using Phase = TimerConfiguration<TimerModel::Definition, 1000, 16000000, CompareSettings>;
using Fixed = TimerConfiguration<TimerModel::Definition, 1000, 16000000, FixedSettings>;

TEST_CASE_METHOD(Fixture, "programmable timer configuration preserves unrelated fields", "[avr]") {
  Memory::bytes[0] = 8;
  TimerModel::Definition::Registers::ReadModifyWrite(TimerModel::CountBits{123}, TimerModel::CompareBits{456});
  Memory::events.clear();
  Capture::setupTimer();
  CHECK(Memory::bytes[0] == 0xe9);
  CHECK(Capture::get_top_count() == 16000);
  CHECK(Capture::getFrequency<std::uint32_t>() == 1000);
  CHECK(Capture::setFrequency(2000) == 8000);
  CHECK(Capture::get_top_count() == 8000);
  CHECK(Capture::getFrequency<std::uint32_t>() == 2000);
  for (const auto& event : Memory::events) {
    CHECK((event.address == 0 || event.address == 4));
    CHECK(event.width == (event.address == 0 ? 1 : 2));
  }
  CHECK(TimerModel::Definition::Registers::Read<TimerModel::CountBits>() == 123);
  CHECK(TimerModel::Definition::Registers::Read<TimerModel::CompareBits>() == 456);
}

TEST_CASE_METHOD(Fixture, "phase correct configuration uses OCRA and half clock", "[avr]") {
  Phase::setupTimer();
  CHECK(Memory::bytes[0] == 0xb1);
  CHECK(Phase::get_top_count() == 8000);
  CHECK(Phase::setFrequency(500) == 16000);
  CHECK(Phase::getFrequency<std::uint32_t>() == 500);
  for (const auto& event : Memory::events) {
    CHECK((event.address == 0 || event.address == 6));
  }
}

TEST_CASE_METHOD(Fixture, "built in configuration applies mode and divider without writing TOP", "[avr]") {
  Memory::bytes[0] = 8;
  Fixed::setupTimer();
  CHECK(Memory::bytes[0] == 0x5b);
  CHECK(Fixed::get_top_count() == 255);
  CHECK(Fixed::getFrequency<std::uint32_t>() == 980);
  CHECK(Fixed::getFrequency<double>() == 16000000.0 / 64 / 255);
  CHECK(Fixed::setFrequency(100) == 255);
  CHECK(Memory::bytes[0] == 0x5d);
  CHECK(Fixed::getFrequency<std::uint32_t>() == 61);
  for (const auto& event : Memory::events) {
    CHECK(event.address == 0);
    CHECK(event.width == 1);
  }
}

TEST_CASE_METHOD(Fixture, "invalid dynamic timer requests perform no register IO", "[avr]") {
  Capture::setupTimer();
  const auto before = Memory::bytes;
  Memory::events.clear();
  for (auto frequency : {0, -1, 16000000, 16000001}) {
    CHECK(Capture::setFrequency(frequency) == 0);
  }
  for (auto frequency : {0.0, -1.0, 0.00001, std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::quiet_NaN()}) {
    CHECK(Capture::setFrequency(frequency) == 0);
    CHECK(Fixed::setFrequency(frequency) == 0);
  }
  CHECK(Fixed::setFrequency(0) == 0);
  CHECK(Fixed::setFrequency(1) == 0);
  CHECK(Fixed::setFrequency(16000001) == 0);
  CHECK(Memory::events.empty());
  CHECK(Memory::bytes == before);
}

TEST_CASE_METHOD(Fixture, "configuration respects a narrower TOP field", "[avr]") {
  using Narrow = Model<Memory, std::uint16_t, std::uint8_t>;
  using Config = TimerConfiguration<Narrow::Definition, 1000, 16000000, CaptureSettings>;
  Config::setupTimer();
  CHECK(Config::get_top_count() == 250);
  CHECK(Memory::bytes[0] == 0xe3);
  CHECK(Config::setFrequency(2000) == 125);
  CHECK(Config::getFrequency<std::uint32_t>() == 2000);
  for (const auto& event : Memory::events) {
    CHECK(event.width == 1);
  }
  Memory::events.clear();
  CHECK(Config::setFrequency(1) == 0);
  CHECK(Memory::events.empty());
}

TEST_CASE_METHOD(Fixture, "frequency reads check live mode and optional TOP metadata", "[avr]") {
  // Unknown mode retains the legacy -1 sentinel, including unsigned conversion.
  Memory::bytes[0] = 0xd1;
  CHECK(Capture::getFrequency<int>() == -1);
  CHECK(Capture::getFrequency<std::uint32_t>() == std::numeric_limits<std::uint32_t>::max());
  Memory::bytes[0] = 0x51;
  CHECK(Capture::getFrequency<std::uint32_t>() == 62745);
  Memory::bytes[0] = 0x50; // Stopped clock is zero frequency.
  CHECK(Capture::getFrequency<int>() == 0);
  Capture::setupTimer();
  Memory::events.clear();
  CHECK(Capture::Config::getFrequency<std::uint32_t>(setl::Optional<std::uint32_t>{2000}, 8000000) == 4000);
  for (const auto& event : Memory::events) {
    CHECK(event.address == 0); // Explicit TOP avoids reading ICR.
  }
  // A mode may name a TOP source the supplied definition does not expose.
  struct WithoutTop : TimerModel::Definition {
    static setl::Optional<std::uint32_t> get_timer_top(TimerTop, EnumWGM1) { return {}; }
  };
  using Config = TimerPwmConfigutation<WithoutTop, 1000, 16000000, TimerPwmMode::fast, TimerTop::icr>;
  CHECK(Config::getFrequency<int>() == -1);
}
