#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>

namespace {
using namespace avr_mock;
static_assert(std::is_same_v<Pin::dependencies, std::tuple<Pin>>);
static_assert(Pin::resource_type == avr::base::ResourceType::digital_gpio);
}

TEST_CASE_METHOD(Fixture, "GPIO input configuration updates direction before latch", "[avr]") {
  Memory::bytes[0x24] = 0xa5;
  Memory::bytes[0x25] = 0x55;
  Pin::configure_input_pullup();
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::BarrierEnter, 0, 0, 0},
    {Kind::Read, 0x24, 1, 0xa5}, {Kind::Write, 0x24, 1, 0x25},
    {Kind::Read, 0x25, 1, 0x55}, {Kind::Write, 0x25, 1, 0xd5},
    {Kind::BarrierExit, 0, 0, 0}});
  Pin::configure_input();
  REQUIRE(Memory::bytes[0x24] == 0x25);
  REQUIRE(Memory::bytes[0x25] == 0x55);
}

TEST_CASE_METHOD(Fixture, "GPIO output configuration sets latch before direction", "[avr]") {
  Memory::bytes[0x24] = 0x25;
  Memory::bytes[0x25] = 0x55;
  Pin::configure_output_true();
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::BarrierEnter, 0, 0, 0},
    {Kind::Read, 0x25, 1, 0x55}, {Kind::Write, 0x25, 1, 0xd5},
    {Kind::Read, 0x24, 1, 0x25}, {Kind::Write, 0x24, 1, 0xa5},
    {Kind::BarrierExit, 0, 0, 0}});
  Pin::configure_output_false();
  REQUIRE(Memory::bytes[0x25] == 0x55);
  REQUIRE(Memory::bytes[0x24] == 0xa5);
  Pin::set(true);
  Pin::configure_output();
  REQUIRE(Memory::bytes[0x25] == 0xd5);
}

TEST_CASE_METHOD(Fixture, "dynamic GPIO configuration matches the corresponding typed operation", "[avr]") {
  for (bool output : {false, true}) {
    for (bool level : {false, true}) {
      Memory::reset(0x55);
      if (output) {
        if (level) { Pin::configure_output_true(); }
        else { Pin::configure_output_false(); }
      } else {
        if (level) { Pin::configure_input_pullup(); }
        else { Pin::configure_input(); }
      }
      const auto expected = Memory::events;
      const auto expected_bytes = Memory::bytes;
      Memory::reset(0x55);
      Pin::configure(output, level);
      CHECK(Memory::events == expected);
      CHECK(Memory::bytes == expected_bytes);
    }
  }
}

TEST_CASE_METHOD(Fixture, "GPIO reads PIN and writes PORT without changing direction", "[avr]") {
  Memory::bytes[0x24] = 0xa5;
  Memory::bytes[0x25] = 0x55;
  Memory::bytes[0x23] = 0x80;
  REQUIRE(Pin::get());
  Pin::set(false);
  REQUIRE(Pin::get());
  Memory::bytes[0x23] = 0;
  REQUIRE_FALSE(Pin::get());
  REQUIRE(Memory::bytes[0x24] == 0xa5);
  REQUIRE(Memory::bytes[0x25] == 0x55);
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::Read, 0x23, 1, 0x80},
    {Kind::Read, 0x25, 1, 0x55}, {Kind::Write, 0x25, 1, 0x55},
    {Kind::Read, 0x23, 1, 0x80}, {Kind::Read, 0x23, 1, 0}});
}

TEST_CASE_METHOD(Fixture, "directional GPIO wrappers configure their declared defaults", "[avr]") {
  using Input = avr::base::InputGpioPort<Pin, true>;
  using Output = avr::base::OutputGpioPort<Pin, true>;
  Input::configure();
  REQUIRE(Memory::bytes[0x24] == 0);
  REQUIRE(Memory::bytes[0x25] == 0x80);
  Output::configure();
  REQUIRE(Memory::bytes[0x24] == 0x80);
  REQUIRE(Memory::bytes[0x25] == 0x80);
  Output::set(false);
  REQUIRE(Memory::bytes[0x25] == 0);
  Memory::bytes[0x23] = 0x80;
  REQUIRE(Input::get());
  REQUIRE(Output::get());
}

TEST_CASE_METHOD(Fixture, "bidirectional GPIO drives low and releases to input", "[avr]") {
  using Bidi = avr::base::BidirectionalGpioPort<Pin>;
  Bidi::configure();
  REQUIRE(Memory::bytes[0x24] == 0);
  REQUIRE(Memory::bytes[0x25] == 0x80);
  Bidi::set(false);
  REQUIRE(Memory::bytes[0x24] == 0x80);
  REQUIRE(Memory::bytes[0x25] == 0);
  Memory::events.clear();
  Bidi::set(true);
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::BarrierEnter, 0, 0, 0},
    {Kind::Read, 0x24, 1, 0x80}, {Kind::Write, 0x24, 1, 0},
    {Kind::Read, 0x25, 1, 0}, {Kind::Write, 0x25, 1, 0x80},
    {Kind::BarrierExit, 0, 0, 0}});
  avr::base::BidirectionalGpioPort<Pin, false>::configure();
  REQUIRE(Memory::bytes[0x25] == 0);
  Memory::bytes[0x23] = 0x80;
  REQUIRE(Bidi::get());
}
