#include "timer_clock_fixture.hpp"
#include "gpio_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
#include <array>

namespace {
using namespace ardo::sys::avr::base;
enum class SparseCode { stopped = 0, unit = 9, eight = 2, sixty_four = 7 };
using SparseMap = DividerMappings<DividerMapping<SparseCode, SparseCode::unit, 1>,
  DividerMapping<SparseCode, SparseCode::eight, 8>, DividerMapping<SparseCode, SparseCode::sixty_four, 64>>;
struct SparseTraits {
  using FreqMapping = SparseMap;
  static constexpr SparseCode null_value = SparseCode::stopped;
};
}

TEST_CASE("clock lookup distinguishes internal and unmapped selectors", "[avr]") {
  constexpr std::array<unsigned, 5> divisors{1, 8, 64, 256, 1024};
  for (unsigned code = 1; code <= 5; ++code) {
    CHECK(findDividerMultiple(static_cast<EnumCS1>(code)) == divisors[code - 1]);
  }
  CHECK(findDividerMultiple(EnumCS1::no_clk) == InvalidClockDivider);
  CHECK(findDividerMultiple(EnumCS1::ext_clk_falling) == InvalidClockDivider);
  CHECK(findDividerMultiple(EnumCS1::ext_clk_rising) == InvalidClockDivider);
  CHECK(findDividerMultiple(static_cast<EnumCS1>(255)) == InvalidClockDivider);
}

TEST_CASE("divider selection rounds required ratios upwards", "[avr]") {
  // A one-step divider would require 256 counts in an eight-bit comparator.
  CHECK(getClockDivider<EnumCS1>(1, 256, 8, false) == EnumCS1::clk8);
  // Just above the eight-step boundary, the next available divider is 64.
  CHECK(getClockDivider<EnumCS1>(1, 2041, 8, false) == EnumCS1::clk64);
  CHECK(getClockDivider<EnumCS1>(1, 261121, 8, false) == EnumCS1::no_clk);
}

TEST_CASE("timer clock helpers retain documented count examples", "[avr]") {
  CHECK(getClockDivider<EnumCS1>(2, 16000000, 16, true) == EnumCS1::clk64);
  CHECK(getClockDivider<EnumCS1>(2, 16000000, 16, false) == EnumCS1::clk256);
  CHECK(getClockDivider<EnumCS2>(2, 16000000, 16, false) == EnumCS2::clk128);
  CHECK(getClockTimerTop(EnumCS1::clk1, 100000, 16000000, true) == 80);
  CHECK(getClockTimerTop(EnumCS1::clk1024, 0.5, 16000000, true) == 15625);
  CHECK(getClockTimerTop(EnumCS1::no_clk, 1, 16000000, true) == 8000000);
  CHECK(getTimerFrequency<double>(15625, EnumCS1::clk1024, 16000000, true) == 0.5);
}

TEST_CASE_METHOD(avr_mock::Fixture, "computed clock values apply through explicit mock registers", "[avr]") {
  using ClockBits = setl::BitsRW<EnumCS1, 2, 1, 0>;
  using TopBits = setl::BitsRW<std::uint16_t>;
  using ClockDef = ardo::sys::avr::nfp::MemRegisterDef<std::uint8_t, 1>;
  using TopDef = ardo::sys::avr::nfp::MemRegisterDef<std::uint16_t, 3>;
  using ClockReg = Register<setl::BitFields<ClockBits>, ClockDef, avr_mock::Binding::IoAccessor>;
  using TopReg = Register<setl::BitFields<TopBits>, TopDef, avr_mock::Binding::IoAccessor>;
  using Selection = setl::RegisterSelector<std::tuple<ClockReg, TopReg>>;
  constexpr auto clock = getClockDivider<EnumCS1>(2, 16000000, 16, false);
  constexpr auto top = getClockTimerTop(clock, 2, 16000000, false);
  using Values = setl::ApplierValues<setl::ApplierValue<ClockBits, clock>,
    setl::ApplierValue<TopBits, top>>;
  avr_mock::Memory::bytes[1] = 0xa0;
  Values::apply<Selection>();
  REQUIRE(avr_mock::Memory::events == std::vector<avr_mock::Event>{
    {avr_mock::Kind::Write, 3, 2, 31250},
    {avr_mock::Kind::Read, 1, 1, 0xa0}, {avr_mock::Kind::Write, 1, 1, 0xa4}});
}

TEST_CASE("divider selection agrees with an independent integer capacity model", "[avr]") {
  constexpr std::array<unsigned, 7> divisors{1, 8, 32, 64, 128, 256, 1024};
  for (bool phase : {false, true}) {
    for (unsigned bits : {8u, 16u, 32u}) {
      for (unsigned frequency : {1u, 2u, 3u, 7u, 61u, 1000u, 100000u}) {
        const std::uint64_t capacity = (std::uint64_t{1} << bits) - 1;
        EnumCS2 expected = EnumCS2::no_clk;
        for (unsigned index = 0; index < divisors.size(); ++index) {
          if (std::uint64_t{frequency} * (phase ? 2 : 1) * divisors[index] * capacity >= 16000000) {
            expected = static_cast<EnumCS2>(index + 1);
            break;
          }
        }
        CHECK(getClockDivider<EnumCS2>(frequency, 16000000, bits, phase) == expected);
      }
    }
  }
}

TEST_CASE("clock arithmetic rejects invalid and unrepresentable requests", "[avr]") {
  const auto nan = std::numeric_limits<double>::quiet_NaN();
  const auto infinity = std::numeric_limits<double>::infinity();
  for (double frequency : {0.0, -1.0, nan, infinity}) {
    CHECK(getClockDivider<EnumCS1>(frequency, 16000000, 16, false) == EnumCS1::no_clk);
    CHECK(getClockTimerTop(EnumCS1::clk1, frequency, 16000000, false) == 0);
    CHECK(getTimerFrequency<double>(frequency, EnumCS1::clk1, 16000000, false) == 0);
  }
  CHECK(getClockDividerMultiple(1, 16000000, 0, false) == InvalidClockDivider);
  CHECK(getClockDividerMultiple(1, 16000000, 33, false) == InvalidClockDivider);
  CHECK(getClockDividerMultiple(1, 0, 16, false) == InvalidClockDivider);
  CHECK(getClockDividerMultiple(16000001, 16000000, 16, false) == InvalidClockDivider);
  CHECK(getClockDividerMultiple(1e-100, 16000000, 16, false) == InvalidClockDivider);
  CHECK(getClockTimerTop(EnumCS1::clk1, 1e-100, 16000000, false) == 0);
  CHECK(getTimerFrequency<double>(1, EnumCS1::no_clk, 16000000, false) == 0);
  CHECK(getTimerFrequency<std::uint8_t>(1, EnumCS1::clk1, 16000000, false) == 0);
  CHECK(getClockTimerTop(EnumCS1::clk1, 1, 0, false) == 0);
}

TEST_CASE("explicit clock traits support sparse and nonmonotonic selector encodings", "[avr]") {
  using Code = SparseCode;
  using Map = SparseMap;
  using Traits = SparseTraits;
  CHECK(getClockDivider<Code, Traits>(1, 256, 8, false) == Code::eight);
  CHECK(Map::findDividerMultiple(Code::unit) == 1);
  CHECK(Map::findDividerMultiple(Code::eight) == 8);
  CHECK(Map::findDividerMultiple(Code::sixty_four) == 64);
  CHECK(Map::findDividerMultiple(static_cast<Code>(1)) == InvalidClockDivider);
  CHECK(findDividerMultiple<Code, Traits>(Code::eight) == 8);
  CHECK(getClockTimerTop<Code, Traits>(Code::eight, 1, 2048, false) == 256);
  CHECK(getTimerFrequency<double, Code, Traits>(256, Code::eight, 2048, false) == 1);
}

TEST_CASE("integer timer arithmetic handles odd clocks and full width boundaries", "[avr]") {
  constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
  for (bool phase : {false, true}) {
    for (std::uint32_t clock : {1u, 2u, 3u, 509u, 511u, maximum - 1, maximum}) {
      for (unsigned bits : {1u, 8u, 16u, 32u}) {
        for (std::uint32_t frequency : {1u, 2u, 3u, 65535u, maximum}) {
          auto expected = InvalidClockDivider;
          if (frequency <= clock / (phase ? 2 : 1)) {
            // Wide arithmetic is confined to this independent host oracle.
            const auto capacity = (std::uint64_t{1} << bits) - 1;
            const auto denominator = std::uint64_t{frequency} * (phase ? 2 : 1) * capacity;
            expected = static_cast<std::uint32_t>(clock / denominator + (clock % denominator != 0));
          }
          CHECK(getClockDividerMultiple(frequency, clock, bits, phase) == expected);
        }
      }
      for (std::uint32_t count : {1u, 2u, 3u, 65535u, maximum}) {
        const auto expected = clock / (std::uint64_t{count} * (phase ? 2 : 1) * 8);
        CHECK(getTimerFrequency<std::uint32_t>(count, EnumCS1::clk8, clock, phase) == expected);
        CHECK(getClockTimerTop(EnumCS1::clk8, count, clock, phase) == expected);
      }
    }
  }
  CHECK(getClockDividerMultiple(1, 511, 8, true) == 2);
  CHECK(getClockTimerTop(EnumCS1::clk1, 1, maximum, false) == maximum);
  CHECK(getTimerFrequency<std::uint32_t>(1, EnumCS1::clk1, maximum, false) == maximum);
}

TEST_CASE("integer timer conversions retain sign range and fractional bounds", "[avr]") {
  constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
  for (std::int64_t value : {-1LL, 0LL, 4294967296LL}) {
    CHECK(getClockDividerMultiple(value, maximum, 16, false) == InvalidClockDivider);
    CHECK(getClockTimerTop(EnumCS1::clk1, value, maximum, false) == 0);
    CHECK(getTimerFrequency<std::uint32_t>(value, EnumCS1::clk1, maximum, false) == 0);
  }
  CHECK(getClockDividerMultiple(std::uint64_t{1} << 32, maximum, 16, false) == InvalidClockDivider);
  CHECK(getTimerFrequency<std::uint8_t>(2, EnumCS1::clk1, 510, false) == 255);
  CHECK(getTimerFrequency<std::uint8_t>(2, EnumCS1::clk1, 511, false) == 0);
  CHECK(getTimerFrequency<std::int8_t>(2, EnumCS1::clk1, 254, false) == 127);
  CHECK(getTimerFrequency<std::int8_t>(2, EnumCS1::clk1, 255, false) == 0);
  CHECK(getTimerFrequency<std::uint8_t>(1, EnumCS1::clk1, 511, true) == 0);
  CHECK(getTimerFrequency<std::uint8_t>(1, EnumCS1::clk8, 2041, false) == 0);
}

TEST_CASE("explicit floating timer paths reject rounded integer overflow bounds", "[avr]") {
  constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
  CHECK(getClockTimerTop(EnumCS1::clk1, 1.0f, maximum, false) == 0);
  CHECK(getTimerFrequency<std::uint32_t>(1.0f, EnumCS1::clk1, maximum, false) == 0);
  CHECK(getClockDividerMultiple(1.0f, maximum, 1, false) == InvalidClockDivider);
  CHECK(getClockTimerTop(EnumCS1::clk1, 1.0, maximum, false) == maximum);
  CHECK(getTimerFrequency<std::uint32_t>(1.0, EnumCS1::clk1, maximum, false) == maximum);
  CHECK(getTimerFrequency<std::int8_t>(1.0f, EnumCS1::clk1, 128, false) == 0);
  CHECK(getTimerFrequency<std::uint8_t>(1.0f, EnumCS1::clk1, 255, false) == 255);
  CHECK(getTimerFrequency<float>(3, EnumCS1::clk1, 3, true) == 0.5f);
  CHECK(getClockTimerTop(EnumCS1::clk8, 0.5f, 2048, false) == 512);
}
