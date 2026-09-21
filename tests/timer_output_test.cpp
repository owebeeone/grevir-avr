#include "timer_output_fixture.hpp"
#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
using namespace timer_output_fixture;
using avr_mock::Fixture;
using avr_mock::Memory;
using Model16 = Model<Memory>;
using Config = Model16::Config;
using A = Config::OutputPin<SettingsA>;
using B = Config::OutputPin<SettingsB>;
using Pins = TimerPwmPinConfiguration<Config, SettingsA, SettingsB>;
using Registers = Model16::Definition::Registers;

static std::vector<std::ptrdiff_t> writes() {
  std::vector<std::ptrdiff_t> result;
  for (const auto& event : Memory::events) {
    if (event.kind == avr_mock::Kind::Write) {
      result.push_back(event.address);
    }
  }
  return result;
}

TEST_CASE_METHOD(Fixture, "timer outputs preload compare and latch before enabling control", "[avr]") {
  Memory::bytes[8] = 0xf0;
  Memory::bytes[16] = 0xc0;
  Memory::bytes[17] = 0x80;
  A::setup(std::uint16_t{4000}, std::uint16_t{16000});
  CHECK(writes() == std::vector<std::ptrdiff_t>{6, 8, 17});
  CHECK(Registers::Read<Model16::CompareABits>() == 4000);
  CHECK(Memory::bytes[8] == 0xf2);
  CHECK(Memory::bytes[17] == 0x81);
  Memory::events.clear();
  B::setup(std::uint16_t{0}, std::uint16_t{16000});
  CHECK(writes() == std::vector<std::ptrdiff_t>{16, 8, 17});
  CHECK(Memory::bytes[16] == 0xc2); // Legacy false polarity: high at zero duty.
  CHECK(Memory::bytes[17] == 0x83);
  CHECK(Memory::bytes[8] == 0xf2);
}

TEST_CASE_METHOD(Fixture, "timer output endpoints preserve polarity and the other channel", "[avr]") {
  Memory::bytes[8] = 0xfc;
  A::pwmWrite(300, std::uint16_t{1000});
  CHECK(Memory::bytes[8] == 0xfe);
  A::pwmWrite(-1, std::uint16_t{1000});
  CHECK(Memory::bytes[8] == 0xfc);
  CHECK((Memory::bytes[16] & 1) == 0);
  A::pwmWrite(1000, std::uint16_t{1000});
  CHECK((Memory::bytes[16] & 1) == 1);
  B::pwmWrite(1001, std::uint16_t{1000});
  CHECK((Memory::bytes[16] & 2) == 0);
  B::pwmWrite(0, std::uint16_t{1000});
  CHECK((Memory::bytes[16] & 2) == 2);
  CHECK(Memory::bytes[8] == 0xf0);
  CHECK(Registers::Read<Model16::CompareABits>() == 300);
}

TEST_CASE_METHOD(Fixture, "fractional output validates before conversion and uses bounded counts", "[avr]") {
  A::pwmWritef(0.25f, std::uint16_t{1000});
  CHECK(Registers::Read<Model16::CompareABits>() == 250);
  A::pwmWritef(0.00001f, std::uint16_t{1000});
  CHECK((Memory::bytes[8] & 3) == 0); // Quantized zero is a GPIO endpoint.
  A::pwmWritef(-100.0f, std::uint16_t{1000});
  CHECK((Memory::bytes[16] & 1) == 0);
  A::pwmWritef(100.0f, std::uint16_t{1000});
  CHECK((Memory::bytes[16] & 1) == 1);
  Memory::events.clear();
  for (float value : {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
      -std::numeric_limits<float>::infinity()}) {
    A::pwmWritef(value, std::uint16_t{1000});
    A::pwmWrite(value, std::uint16_t{1000});
  }
  A::pwmWritef(0.5f, 0);
  A::pwmWrite(10, 65536u);
  A::setup(10, 0);
  CHECK(Memory::events.empty());
}

TEST_CASE_METHOD(Fixture, "duty rescaling uses exact integer ratios at sixteen bit boundaries", "[avr]") {
  A::pwmWrite(65534u, 65535u);
  A::pwmAdjust(65535u, 65534u);
  CHECK(Registers::Read<Model16::CompareABits>() == 65533);
  A::pwmWrite(1, 3);
  A::pwmAdjust(3, 1000);
  CHECK(Registers::Read<Model16::CompareABits>() == 333);
  A::pwmWrite(1, 1000);
  A::pwmAdjust(1000, 3);
  CHECK((Memory::bytes[8] & 3) == 0);
  CHECK((Memory::bytes[16] & 1) == 0);
  // A stale compare above the old TOP saturates to the new full-duty endpoint.
  A::setupTimerOutputMode();
  A::pwmWriteAbsoluteValue(500);
  A::pwmAdjust(100, 200);
  CHECK((Memory::bytes[16] & 1) == 1);
  CHECK((Memory::bytes[8] & 3) == 0);
}

TEST_CASE_METHOD(Fixture, "duty adjustment leaves disconnected and unrelated modes unchanged", "[avr]") {
  A::pwmWrite(0, 1000);
  auto before = Memory::bytes;
  Memory::events.clear();
  A::pwmAdjust(1000, 500);
  CHECK(writes().empty());
  CHECK(Memory::bytes == before);
  Memory::bytes[8] = 1; // Toggle mode is not this pin's configured PWM mode.
  Memory::events.clear();
  A::pwmAdjust(1000, 500);
  CHECK(writes().empty());
  Memory::events.clear();
  A::pwmAdjust(0, 500);
  A::pwmAdjust(1000, 0);
  A::pwmAdjust(1000, 1000);
  A::pwmAdjust(1000, 65536);
  CHECK(Memory::events.empty());
}

TEST_CASE_METHOD(Fixture, "composed PWM outputs retain duty across valid frequency updates", "[avr]") {
  Pins::setup();
  Pins::pwmWrite(SettingsA{}, 4000);
  Pins::pwmWrite(SettingsB{}, 12000);
  CHECK(Registers::Read<Model16::CompareABits>() == 4000);
  CHECK(Registers::Read<Model16::CompareBBits>() == 12000);
  CHECK(Pins::setFrequency(2000) == 7999);
  CHECK(Registers::Read<Model16::CompareABits>() == 1999);
  CHECK(Registers::Read<Model16::CompareBBits>() == 5999);
  CHECK(Pins::getFrequency<std::uint32_t>() == 2000);
  Pins::pwmWrite(SettingsB{}, 0);
  CHECK(Pins::setFrequency(4000) == 3999);
  CHECK((Memory::bytes[8] & 12) == 0);
  CHECK((Memory::bytes[16] & 2) == 2);
  const auto before = Memory::bytes;
  Memory::events.clear();
  CHECK(Pins::setFrequency(0) == 0);
  CHECK(writes().empty()); // The composed wrapper reads the previous TOP only.
  CHECK(Memory::bytes == before);
}

TEST_CASE_METHOD(Fixture, "timer facade reads checked live TOP and handles no output pins", "[avr]") {
  using Facade = Timer<Model16::Definition>;
  using Empty = Facade::PwmPinConfiguration<Config>;
  Empty::setup();
  CHECK(Facade::getTopCount() == 15999);
  CHECK(Empty::setFrequency(2000) == 7999);
  CHECK(Empty::get_top_count() == 7999);
  CHECK((Memory::bytes[17] & 3) == 0);
  Memory::bytes[0] = 0x51;
  CHECK(Facade::getTopCount() == 255);
  Memory::events.clear();
  Memory::bytes[0] = 0xd1;
  CHECK(Facade::getTopCount() == 0);
  CHECK(Memory::events.size() == 1);
}

TEST_CASE_METHOD(Fixture, "eight bit outputs and OCRA TOP apply only the selected output", "[avr]") {
  using Small = Model<Memory, std::uint8_t>;
  using SmallPin = Small::Config::OutputPin<SettingsB>;
  SmallPin::pwmWrite(200, 250);
  SmallPin::pwmAdjust(250, 125);
  CHECK(Small::Definition::Registers::Read<Small::CompareBBits>() == 100);
  for (const auto& event : Memory::events) {
    CHECK(event.width == 1);
  }
  using OcrConfig = TimerConfiguration<Model16::Definition, 1000, 16000000,
    timer_configuration_fixture::CompareSettings>;
  using OcrPins = TimerPwmPinConfiguration<OcrConfig, SettingsB>;
  OcrPins::setup();
  OcrPins::pwmWrite(SettingsB{}, 2000);
  CHECK(Registers::Read<Model16::CompareABits>() == 8000);
  CHECK(Registers::Read<Model16::CompareBBits>() == 2000);
  CHECK(OcrPins::setFrequency(2000) == 4000);
  CHECK(Registers::Read<Model16::CompareABits>() == 4000);
  CHECK(Registers::Read<Model16::CompareBBits>() == 1000);
}
