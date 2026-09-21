#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>

namespace { using namespace avr_mock; }

TEST_CASE_METHOD(Fixture, "AVR IO offset and memory address bindings access the same byte", "[avr]") {
  using Whole = setl::BitsRW<std::uint8_t>;
  using Io = avr::nfp::IoRegisterDef<std::uint8_t, 5, 0x20>;
  using Mem = avr::nfp::MemRegisterDef<std::uint8_t, 0x25>;
  using IoReg = avr::base::Register<setl::BitFields<Whole>, Io, Binding::IoAccessor>;
  using MemReg = avr::base::Register<setl::BitFields<Whole>, Mem, Binding::IoAccessor>;
  IoReg::Write(Whole{0xa5});
  REQUIRE(MemReg::Read().value == 0xa5);
  REQUIRE(Memory::bytes[5] == 0);
  REQUIRE(Memory::bytes[0x45] == 0);
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::Write, 0x25, 1, 0xa5}, {Kind::Read, 0x25, 1, 0xa5}});
}

TEST_CASE_METHOD(Fixture, "AVR alternate register width retains address and explicit offset", "[avr]") {
  using Byte = avr::nfp::IoRegisterDef<std::uint8_t, 5, 0x20>;
  using Word = setl::BitsRW<std::uint16_t>;
  using Reg = avr::base::Register<setl::BitFields<Word>, Byte::ForType<std::uint16_t>, Binding::IoAccessor>;
  Reg::Write(Word{0x1234});
  REQUIRE(Reg::Read().value == 0x1234);
  REQUIRE(Memory::bytes[0x24] == 0);
  REQUIRE(Memory::bytes[0x27] == 0);
  REQUIRE(Memory::events == std::vector<Event>{
    {Kind::Write, 0x25, 2, 0x1234}, {Kind::Read, 0x25, 2, 0x1234}});
}

TEST_CASE_METHOD(Fixture, "AVR address binding accepts a caller chosen zero offset", "[avr]") {
  using Whole = setl::BitsRW<std::uint8_t>;
  using Def = avr::nfp::IoRegisterDef<std::uint8_t, 5, 0>;
  using Reg = avr::base::Register<setl::BitFields<Whole>, Def, Binding::IoAccessor>;
  Reg::Write(Whole{0x42});
  REQUIRE(Memory::events == std::vector<Event>{{Kind::Write, 5, 1, 0x42}});
  REQUIRE(Memory::bytes[0x25] == 0);
}
