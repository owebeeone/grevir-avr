#pragma once

#include <GrevirRegisters.h>
#include <grevir/base/compat/cstdint.hpp>
#include <grevir/base/compat/type_traits.hpp>

namespace ardo::sys::avr::base {

// Direct volatile access adapted from the original McuRegister. Selecting it is
// explicit. It supplies neither interrupt exclusion nor special device semantics.
struct VolatileAccess {
  template <typename T>
  static T read(std::ptrdiff_t address) {
    static_assert(std::is_unsigned_v<T>);
    return *reinterpret_cast<volatile T*>(static_cast<std::uintptr_t>(address));
  }
  template <typename T>
  static void write(std::ptrdiff_t address, T value) {
    static_assert(std::is_unsigned_v<T>);
    *reinterpret_cast<volatile T*>(static_cast<std::uintptr_t>(address)) = value;
  }
  template <typename T>
  static void modify(std::ptrdiff_t address, T value, T mask) {
    write<T>(address, static_cast<T>((read<T>(address) & ~mask) | (value & mask)));
  }
};

// Replaces the global numeric debug-mode switch with an explicit policy type.
template <typename Policy>
struct RegisterSelector {
  template <typename T, std::ptrdiff_t Address>
  using IoAccessor = setl::McuRegister<T, Address, Policy>;
};

template <typename T, std::ptrdiff_t Address, typename Policy>
using IoAccessor = setl::McuRegister<T, Address, Policy>;

template <typename Fields, typename Definition,
          template <typename, std::ptrdiff_t> typename Access>
using Register = setl::IoRegister<Fields, Definition, Access>;

} // namespace ardo::sys::avr::base
