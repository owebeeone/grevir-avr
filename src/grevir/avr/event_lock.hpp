#pragma once

#if defined(__AVR__)
#include <grevir/base/compat/cstdint.hpp>
#include <grevir/base/compat/string_view.hpp>

namespace grevir::avr {

// GCC's AVR assembler names the CPU status register __SREG__. Saving and
// restoring the whole register preserves a caller's prior interrupt state,
// including when this lock is constructed inside an ISR.
class EventLock {
 public:
  inline static constexpr std::string_view identity{"avr_sreg_v1", 11};
  EventLock() noexcept {
    asm volatile("in %0, __SREG__\n\tcli" : "=r"(saved_) :: "cc", "memory");
  }
  ~EventLock() noexcept {
    asm volatile("out __SREG__, %0" :: "r"(saved_) : "cc", "memory");
  }
  EventLock(const EventLock&) = delete;
  EventLock& operator=(const EventLock&) = delete;

 private:
  std::uint8_t saved_{};
};

} // namespace grevir::avr
#endif
