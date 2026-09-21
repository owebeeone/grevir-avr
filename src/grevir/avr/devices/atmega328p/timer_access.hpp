#pragma once

#include <grevir/avr/generated/atmega328p/timer_gpio.hpp>
#include <grevir/base/compat/cstdint.hpp>
#include <grevir/base/compat/type_traits.hpp>

namespace ardo::sys::avr::arch_atmega328p::nfp {

// Adapter for the timer register inventory, not a general AVR memory accessor.
// ByteAccess receives memory addresses (the IO offset has already been applied).
// Barrier must preserve/restore interrupt state; the caller provides its policy.
template <typename ByteAccess, typename Barrier>
struct TimerAccess {
 private:
  static bool timerWord(std::ptrdiff_t address) {
    return address == rrTCNT1::addr || address == rrICR1::addr
      || address == rrOCR1A::addr || address == rrOCR1B::addr;
  }
  static bool flags(std::ptrdiff_t address) {
    return address == rrTIFR0::addr || address == rrTIFR1::addr || address == rrTIFR2::addr;
  }
  static std::uint16_t readPair(std::ptrdiff_t address) {
    const auto low = ByteAccess::template read<std::uint8_t>(address);
    const auto high = ByteAccess::template read<std::uint8_t>(address + 1);
    return static_cast<std::uint16_t>(low | (std::uint16_t{high} << 8));
  }
  static void writePair(std::ptrdiff_t address, std::uint16_t value) {
    const auto low = static_cast<std::uint8_t>(value);
    const auto high = static_cast<std::uint8_t>(value >> 8);
    if (timerWord(address)) {
      // Timer1's shared TEMP latch: high first, then low commits both bytes.
      ByteAccess::template write<std::uint8_t>(address + 1, high);
      ByteAccess::template write<std::uint8_t>(address, low);
    } else {
      // Synthetic TCCRnA/B pair: two independent byte registers, A then B.
      ByteAccess::template write<std::uint8_t>(address, low);
      ByteAccess::template write<std::uint8_t>(address + 1, high);
    }
  }
  static void modifyByte(std::ptrdiff_t address, std::uint8_t value, std::uint8_t mask) {
    if (mask != 0) {
      if (flags(address)) {
        // W1C: never echo another pending flag back through a read/modify/write.
        ByteAccess::template write<std::uint8_t>(address, static_cast<std::uint8_t>(value & mask));
      } else {
        ByteAccess::template modify<std::uint8_t>(address, value, mask);
      }
    }
  }

 public:
  template <typename T>
  static T read(std::ptrdiff_t address) {
    static_assert(std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t>);
    if constexpr (sizeof(T) == 1) {
      return ByteAccess::template read<std::uint8_t>(address);
    } else {
      Barrier barrier;
      return readPair(address);
    }
  }
  template <typename T>
  static void write(std::ptrdiff_t address, T value) {
    static_assert(std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t>);
    if constexpr (sizeof(T) == 1) {
      ByteAccess::template write<std::uint8_t>(address, value);
    } else {
      Barrier barrier;
      writePair(address, value);
    }
  }
  template <typename T>
  static void modify(std::ptrdiff_t address, T value, T mask) {
    static_assert(std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t>);
    Barrier barrier;
    if constexpr (sizeof(T) == 1) {
      modifyByte(address, value, mask);
    } else {
      if (timerWord(address)) {
        const auto previous = readPair(address);
        writePair(address, static_cast<std::uint16_t>((previous & ~mask) | (value & mask)));
      } else {
        // Preserve unused control bytes without even reading them.
        modifyByte(address, static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(mask));
        modifyByte(address + 1, static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(mask >> 8));
      }
    }
  }
};
} // namespace ardo::sys::avr::arch_atmega328p::nfp
