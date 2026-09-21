#include "timer_definition_fixture.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace timer_definition_mock;

TEST_CASE_METHOD(Fixture, "timer capture settings preserve unrelated control bits", "[avr]") {
  Memory::bytes[17] = 0x15;
  Capture::NoiseCanceller::Activate::apply<Definition::Registers>();
  CHECK(Memory::bytes[17] == 0x95);
  Capture::Rising::apply<Definition::Registers>();
  CHECK(Memory::bytes[17] == 0xd5);
  Capture::NoiseCanceller::Deactivate::apply<Definition::Registers>();
  CHECK(Memory::bytes[17] == 0x55);
  Capture::Falling::apply<Definition::Registers>();
  CHECK(Memory::bytes[17] == 0x15);
  REQUIRE(Memory::events.size() == 8);
  for (const auto& event : Memory::events) {
    CHECK(event.address == 17);
    CHECK(event.width == 1);
  }
}

TEST_CASE_METHOD(Fixture, "absent capture and noise cancellation perform no IO", "[avr]") {
  NoCapture::Rising::apply<NoCaptureDefinition::Registers>();
  NoCapture::Falling::apply<NoCaptureDefinition::Registers>();
  NoCapture::NoiseCanceller::Activate::apply<NoCaptureDefinition::Registers>();
  NoCapture::NoiseCanceller::Deactivate::apply<NoCaptureDefinition::Registers>();
  using WithoutNoise = TimerCapture<CaptureBits, void, EdgeBits>;
  WithoutNoise::NoiseCanceller::Activate::apply<Definition::Registers>();
  WithoutNoise::NoiseCanceller::Deactivate::apply<Definition::Registers>();
  CHECK(Memory::events.empty());
}

TEST_CASE_METHOD(Fixture, "timer TOP reads use the selected native width register", "[avr]") {
  Definition::Registers::ReadModifyWrite(CaptureBits{0xabcd}, CompareABits{0x1234}, CompareBBits{0x789a});
  Memory::events.clear();
  auto top = Definition::get_timer_top(TimerTop::icr);
  REQUIRE(top.is_present());
  CHECK(top.get() == 0xabcd);
  REQUIRE(Memory::events == std::vector<avr_mock::Event>{{avr_mock::Kind::Read, 20, 2, 0xabcd}});
  Memory::events.clear();
  top = Definition::get_timer_top(TimerTop::ocra, EnumWGM1::reserved_d);
  REQUIRE(top.is_present());
  CHECK(top.get() == 0x1234);
  REQUIRE(Memory::events == std::vector<avr_mock::Event>{{avr_mock::Kind::Read, 22, 2, 0x1234}});
  // Register TOP queries are independent of the supplied waveform code.
  for (std::uint16_t value : {std::uint16_t{0}, std::uint16_t{65535}}) {
    CompareOnlyDefinition::Registers::ReadModifyWrite(CompareABits{value});
    top = CompareOnlyDefinition::get_timer_top(TimerTop::ocra);
    REQUIRE(top.is_present());
    CHECK(top.get() == value);
  }
}

TEST_CASE_METHOD(Fixture, "built in TOP lookup preserves absent waveform metadata", "[avr]") {
  auto top = Definition::get_timer_top(TimerTop::built_in, EnumWGM1::fast_pwm_9bit);
  REQUIRE(top.is_present());
  CHECK(top.get() == 511);
  CHECK(Memory::events.empty());
  for (auto code : {EnumWGM1::reserved_d, EnumWGM1::fast_pwm_icr, EnumWGM1::ctc_ocra, static_cast<EnumWGM1>(255)}) {
    CHECK_FALSE(Definition::get_timer_top(TimerTop::built_in, code).is_present());
  }
  CHECK(Memory::events.empty());
  Memory::bytes[16] = 0xa7;
  top = Definition::get_timer_top(TimerTop::built_in);
  REQUIRE(top.is_present());
  CHECK(top.get() == 1023);
  REQUIRE(Memory::events == std::vector<avr_mock::Event>{{avr_mock::Kind::Read, 16, 1, 0xa7}});
  Memory::bytes[16] = 0xad;
  CHECK_FALSE(Definition::get_timer_top(TimerTop::built_in).is_present());
}

TEST_CASE_METHOD(Fixture, "unsupported and empty TOP inventories perform no reads", "[avr]") {
  CHECK_FALSE(NoCaptureDefinition::get_timer_top(TimerTop::icr).is_present());
  CHECK_FALSE(CompareOnlyDefinition::get_timer_top(TimerTop::built_in).is_present());
  CHECK_FALSE(Definition::get_timer_top(TimerTop::none).is_present());
  CHECK_FALSE(Definition::get_timer_top(static_cast<TimerTop>(255), EnumWGM1::normal).is_present());
  CHECK_FALSE(NoTopDefinition::get_timer_top(TimerTop::built_in).is_present());
  CHECK_FALSE(NoTopDefinition::get_timer_top(TimerTop::ocra, EnumWGM1::normal).is_present());
  CHECK(Memory::events.empty());
}
