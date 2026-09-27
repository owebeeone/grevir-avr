#include "portable_pwm_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
using namespace portable_pwm_fixture;

TEST_CASE_METHOD(atmega328p_mock::Fixture, "module-owned timer configures once before callbacks", "[avr]") {
  callbacks.clear();
  prerequisite_ready = false;
  writes_after_prerequisite = true;
  custom_params_setup = 0;
  custom_params_loop = 0;
  App::runSetup();
  CHECK(callbacks == std::vector<unsigned>{9,0,1,2});
  CHECK(writes_after_prerequisite);
  CHECK(custom_params_setup == 1);
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
  CHECK(custom_params_loop == 1);
  CHECK(Memory::events.empty());

  Memory::reset();
  callbacks.clear();
  prerequisite_ready = false;
  writes_after_prerequisite = true;
  custom_params_setup = 0;
  Reverse::runSetup();
  CHECK(callbacks == std::vector<unsigned>{9,0,1,2});
  CHECK(writes_after_prerequisite);
  CHECK(custom_params_setup == 1);
}

TEST_CASE_METHOD(atmega328p_mock::Fixture, "independent modules receive distinct physical timers", "[avr]") {
  prerequisite_ready = false;
  TwoOwnerApp::runSetup();
  CHECK(word(0x86) == 15999);
  CHECK(atmega328p_mock::Memory::bytes[0x47] == 0);
  CHECK(atmega328p_mock::Memory::bytes[0x45] != 0);
}
