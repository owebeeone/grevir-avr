#pragma once
#include <grevir/avr/devices/atmega328p/timers.hpp>
#include <grevir/test/register_memory.hpp>

namespace atmega328p_mock {
namespace device = ardo::sys::avr::arch_atmega328p;
namespace base = ardo::sys::avr::base;
using grevir::test::Kind;
using grevir::test::Event;
struct Identity;
using Memory = grevir::test::RegisterMemory<256, Identity>;
struct Bytes {
  static inline std::uint8_t temporary = 0;
  static bool lowWord(std::ptrdiff_t a) { return a == 0x84 || a == 0x86 || a == 0x88 || a == 0x8a; }
  template <typename T>
  static T read(std::ptrdiff_t a) {
    static_assert(std::is_same_v<T, std::uint8_t>);
    if (a == 0x84 || a == 0x86) {
      temporary = Memory::bytes[a + 1];
    }
    if (a == 0x85 || a == 0x87) {
      Memory::events.push_back({Kind::Read, a, 1, temporary});
      return temporary;
    }
    return Memory::read<T>(a);
  }
  template <typename T>
  static void write(std::ptrdiff_t a, T value) {
    static_assert(std::is_same_v<T, std::uint8_t>);
    Memory::events.push_back({Kind::Write, a, 1, value});
    if (lowWord(a - 1)) {
      temporary = value;
    } else if (lowWord(a)) {
      Memory::bytes[a + 1] = temporary;
      Memory::bytes[a] = value;
    } else if (a >= 0x35 && a <= 0x37) {
      Memory::bytes[a] &= static_cast<std::uint8_t>(~value); // W1C interrupt flags.
    } else if (a == 0x45 || a == 0xb1 || a == 0x82) {
      Memory::bytes[a] = static_cast<std::uint8_t>(value & 0x3f); // FOC strobes read as zero.
    } else {
      Memory::bytes[a] = value;
    }
  }
  template <typename T>
  static void modify(std::ptrdiff_t a, T value, T mask) {
    write<T>(a, static_cast<T>((read<T>(a) & ~mask) | (value & mask)));
  }
};
struct Barrier {
  Barrier() { Memory::events.push_back({Kind::BarrierEnter, 0, 0, 0}); }
  ~Barrier() { Memory::events.push_back({Kind::BarrierExit, 0, 0, 0}); }
};
using Bindings = device::TimerBindings<Bytes, Barrier>;
struct Fixture {
  Fixture() { Memory::reset(); Bytes::temporary = 0; }
};
inline std::uint16_t word(std::ptrdiff_t a) {
  return static_cast<std::uint16_t>(Memory::bytes[a] | (std::uint16_t{Memory::bytes[a + 1]} << 8));
}
inline std::vector<Event> io() {
  std::vector<Event> result;
  for (auto e : Memory::events) {
    if (e.kind == Kind::Read || e.kind == Kind::Write) {
      result.push_back(e);
    }
  }
  return result;
}
} // namespace atmega328p_mock
