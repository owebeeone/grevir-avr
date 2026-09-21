# Grevir AVR

AVR register/GPIO and initial timer-clock extraction from Ardoinus. This package depends on Grevir
Registers and Core, retains the `ardo::sys::avr` namespaces, and uses explicit
access and synchronization policies. It includes no Arduino dependency or complete
device inventory. AVR compiler and hardware validation remain on hold.

## Extracted source and interfaces

| Header | Ardoinus source / responsibility |
| --- | --- |
| `register_definitions.hpp` | `ardo_avr_bit_io_defs.h`: memory addresses, caller-supplied I/O offsets and `ForType` |
| `legacy_register_definitions.hpp` | Forwarding replacement for the duplicate `setl_bit_io_defs.h` declarations |
| `register.hpp` | `sys/mcu/avr/base/ardo_avr_base_register.h`: explicit access binding and register alias |
| `gpio.hpp` | Base GPIO header plus directional wrappers from `ardo_supplemental_atmega328p_dev.h` |
| `timer/clock.hpp` | Divider maps and count/frequency computations consolidated from both legacy timer implementations |

All headers live under `grevir/avr/`; `<GrevirAVR.h>` is the public entry point.
The duplicate generic GPIO bodies are consolidated. Core already supplies their
`Dependency`/`ResourceType` graph; no duplicate resource graph is introduced.

`nfp::MemRegisterDef<T, Address>` takes a memory address. The corresponding
`nfp::IoRegisterDef<T, Address, Offset>` adds the explicit offset exactly once;
`ForType<Other>` changes the width while retaining the address. No universal AVR
offset is assumed. The host fixture checks the legacy `5 + 0x20 == 0x25`
arrangement and a zero offset. The caller must supply valid addresses and widths.

`base::RegisterSelector<Policy>::IoAccessor<T, Address>` replaces the old numeric
debug-mode selector. `base::IoAccessor<T, Address, Policy>` is an equivalent direct
binding. `base::Register<Fields, Definition, Access>` requires the access template.
No global `AVR_MOCK_IOREGISTERS` switch or external mock-memory globals are used.

`base::VolatileAccess` provides explicit raw volatile reads/writes and masked
read/modify/write, adapted from the original `McuRegister`. Native checks instantiate
this code but never execute it. It supplies no interrupt exclusion, target barrier,
special multi-byte sequencing or device-specific flag semantics. Target compiler
validation is still pending; ordinary volatile access is not sufficient for every
AVR peripheral register.

## GPIO configuration

`base::GpioPort<Derived, Definition, MemoryBarrier>` requires a caller-supplied RAII
barrier. `GpioPortDefinition` identifies the PORT, PIN and DDR fields/registers.
`InputGpioPort`, `OutputGpioPort` and `BidirectionalGpioPort` retain their legacy
configuration APIs. The bidirectional wrapper switches between low output and
released input, with optional pull-up configuration.

Typed output configuration writes the latch before enabling output; input
configuration disables output before updating the latch. Mock traces exposed that
the old dynamic `configure(bool, bool)` always changed direction first. It now
uses the same order as the corresponding typed operation for all four argument
combinations. `configure_output()` alone retains the current latch value.
`set()` writes PORT; `get()` reads PIN. Unrelated register bits are preserved.

The supplied barrier surrounds configuration, including its constructor and
destructor. Its actual target synchronization guarantees are the caller's
responsibility. Reads and ordinary `set()` retain their separate legacy access
behavior. Device-wide pull-up controls and physical pad behavior are not modeled.

## Timer clock calculations

The divider mappings and five clock helpers are identical in
`ardo_avr_base_timer.h` and `ardo_supplemental_atmega328p_dev.h` after whitespace
normalization. They now have one implementation in `timer/clock.hpp`, under
`ardo::sys::avr::base`, with no concrete device enums or global CPU clock.
The caller supplies clock frequency and either an explicit traits type or a
specialization of the legacy `TccrEnumTraits<Enum>`.

Traits provide `FreqMapping` and `null_value`. Divider tables no longer carry
implicit `start_range`/`null_value` members; that metadata belongs to the caller's
traits. Explicit forms include `getClockDivider<Enum, Traits>(...)`,
`getClockTimerTop<Enum, Traits>(...)`, `findDividerMultiple<Enum, Traits>(...)` and
`getTimerFrequency<Result, Enum, Traits>(...)`. Common legacy calls that name only
the enum/result type still use enum-trait specialization.

Mappings require positive, strictly increasing dividers, unique selectors and a
single enum type; selector numeric codes need not be ordered or contiguous.
Lookup now compares selectors for equality. The old comparison silently treated
the stopped clock as divide-by-one. Unmapped lookup returns `InvalidClockDivider`,
an explicitly 32-bit all-ones sentinel, including stopped/external clock codes.

The required divider now rounds upward before choosing the smallest available
sufficient prescaler. Previously truncation could select a divider that left the
count outside its comparator width: frequency 1, clock 256 and width 8 chose one
instead of eight. The legacy method name `findLargestLessThanOrEq` is retained,
but its actual meaning is the smallest available divider greater than or equal
to the request. No sufficient divider yields the trait's null selector.

Zero/negative/nonfinite frequencies, zero clocks, widths outside 1..32 and
unrepresentable requests are rejected with the invalid-divider/null-selector
sentinel. Count/frequency helpers return zero for invalid or unrepresentable
arithmetic. `getClockTimerTop` deliberately retains its documented unit-divider
fallback for an unmapped selector; callers needing strict validation must check
lookup first. `getTimerFrequency` returns zero for an unmapped selector.

These helpers preserve the legacy count model `clock / phase_factor / divider /
frequency`. The result is not a complete waveform-specific hardware TOP encoding;
mode-dependent adjustments belong to the later mode/configuration extraction.
Integer inputs and integer results use 32-bit integer arithmetic. Divider
selection uses successive ceiling divisions and a 32-bit capacity mask, avoiding
overflowing products while retaining odd clock ticks until rounding is complete.
Count and frequency results use successive floor divisions; narrow result types
still reject values above their maximum, including a maximum plus a fraction.
Explicit floating inputs/results select floating arithmetic at their common
floating type, without unconditional promotion to `long double`. Its precision
therefore follows the selected type. Integer conversion bounds reject rounded
out-of-range values before casting, including `float` rounding `UINT32_MAX` up to
2^32. Target floating-point behavior, size and runtime cost remain unvalidated.
No AVR compiler validation is performed.

## Validation and use

Apple Clang 21 / arm64 macOS / C++23 checks:

- Six public headers compile independently, alongside address/type assertions and
  a compile-only volatile-access user.
- Nineteen host cases pass. They cover
  offsets, widths, preserved bits, access order, explicit barrier scopes, reads,
  directional wrappers and open-drain configuration. Dynamic ordering first failed
  for both output levels and now agrees with typed configuration.
- Eleven legacy and six arithmetic-boundary clock static assertions compile. One valid and seven rejected
  mapping probes check divider values/order, duplicate selectors and mixed enums,
  including faults deeper in a table. Boundary, independent capacity-model and
  invalid-input cases pass; computed values are applied to synthetic registers.
- Added arithmetic cases cover odd clocks, full 32-bit boundaries, signed/wide
  inputs, narrow output limits and explicitly selected floating-point conversion
  bounds. Optimized native IR for dynamic 32-bit divider/count/frequency calls
  contains no floating-point or 64-bit arithmetic; this is not AVR codegen proof.
- Isolated production/host builds and an installed consumer pass. The production
  consumer has Catch2 and Test Support discovery disabled; it supplies its own
  memory policy and exercises explicit clock traits. Test fixtures come from `grevir::test_support` only in host tests.

With Core, Registers and their dependencies installed:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=<prefix> -DGREVIR_BUILD_COMPILE_CHECKS=ON
cmake --build build
cmake --install build --prefix <prefix>
```

Consumers use `find_package(grevir-avr CONFIG REQUIRED)` and link `grevir::avr`.
Host tests opt in with `GREVIR_BUILD_HOST_TESTS=ON` and installed Test Support/Catch2.
Arduino metadata is present; no Arduino sketch or target compilation is claimed.
Concrete MCU pin/register inventories, an AVR barrier policy, waveform modes and
complete timer configuration,
portable pin-backend adaptation and board mappings remain later increments. The
original Ardoinus checkout remains unchanged.
