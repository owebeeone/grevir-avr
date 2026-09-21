#include "portable_pwm_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
using namespace portable_pwm_fixture;

TEST_CASE_METHOD(atmega328p_mock::Fixture, "portable PWM application configures shared owner before module callbacks", "[avr]") {
  callbacks.clear();
  App::runSetup();
  CHECK(callbacks == std::vector<unsigned>{0,1,2});
  CHECK(ready_during_params);
  CHECK(word(0x86) == 15999);
  CHECK(word(0x88) == 3999);
  CHECK(word(0x8a) == 11999);
  unsigned top_writes = 0;
  for (const auto& event : Memory::events) {
    if (event.kind == grevir::test::Kind::Write && event.address == 0x86) { ++top_writes; }
  }
  CHECK(top_writes == 1);
  Memory::events.clear();
  callbacks.clear();
  App::runLoop();
  CHECK(callbacks == std::vector<unsigned>{3,4});
  CHECK(Memory::events.empty());
}
