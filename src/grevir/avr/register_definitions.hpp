#pragma once

#include <cstddef>

namespace ardo::sys::avr::nfp {

/**
 * Defines an IO location for a mapped location.
 */
template <typename T, std::ptrdiff_t w_addr>
struct MemRegisterDef {
  static constexpr std::ptrdiff_t addr = w_addr;
  using type = T;

  /// Provide a MemRegisterDef for an alternate type.
  template <typename Talt>
  using ForType = MemRegisterDef<Talt, addr>;
};

template <typename T, std::ptrdiff_t w_addr, std::ptrdiff_t w_offset>
struct IoRegisterDef {
  static constexpr std::ptrdiff_t offset = w_offset;
  static constexpr std::ptrdiff_t addr = w_addr + offset;
  using type = T;

  /// Provide an IoRegisterDef for an alternate type.
  template <typename Talt>
  using ForType = IoRegisterDef<Talt, w_addr, offset>;
};

} // namespace ardo::sys::avr::nfp
