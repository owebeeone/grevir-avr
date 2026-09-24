# Grevir AVR

**Public API:** [Grevir AVR](https://github.com/owebeeone/grevir-wz/blob/main/docs/api/avr.md).
See [installation](https://github.com/owebeeone/grevir-wz/blob/main/docs/install.md) and
[supported platforms](https://github.com/owebeeone/grevir-wz/blob/main/docs/supported.md).
The workspace `/docs` is the current user-facing contract; development
checkpoints below are historical.

AVR register, GPIO and ATmega328P timer backend implementations.
The first interrupt backend emits the ATmega328P Timer1 overflow vector and
provides mask/pending/enable operations. Its Uno firmware was compiled with
AVR GCC 14.2 and exercised in simavr; physical hardware remains untested.
See the [interrupt guide](https://github.com/owebeeone/grevir-wz/blob/main/docs/guides/interrupts.md).

## Development record (historical)

AVR register/GPIO and timer clock/mode/definition/configuration/output extraction from Ardoinus. This package depends on Grevir
Base, Registers and Core, retains the `ardo::sys::avr` namespaces, and uses explicit
access and synchronization policies. It includes an explicitly selected ATmega328P timer/GPIO binding and no Arduino
dependency. Other peripherals and a full device resource graph remain pending. AVR compiler and hardware validation remain on hold.

## Extracted source and interfaces

| Header | Ardoinus source / responsibility |
| --- | --- |
| `register_definitions.hpp` | `ardo_avr_bit_io_defs.h`: memory addresses, caller-supplied I/O offsets and `ForType` |
| `legacy_register_definitions.hpp` | Forwarding replacement for the duplicate `setl_bit_io_defs.h` declarations |
| `register.hpp` | `sys/mcu/avr/base/ardo_avr_base_register.h`: explicit access binding and register alias |
| `gpio.hpp` | Base GPIO header plus directional wrappers from `ardo_supplemental_atmega328p_dev.h` |
| `timer/clock.hpp` | Divider maps and count/frequency computations consolidated from both legacy timer implementations |
| `timer/mode.hpp` | Waveform metadata, compile-time selection and runtime metadata lookup, with caller-supplied mode tables |
| `timer/definition.hpp` | Capture/noise controls, output-compare metadata, TOP access and composition of a timer's register interfaces |
| `timer/configuration.hpp` | PWM settings, compile-time calculations, setup and checked runtime frequency changes |
| `timer/output.hpp` | PWM output application, duty adjustment, pin composition and the reusable timer facade |
| `generated/atmega328p/timer_gpio.hpp` | Raw timer/GPIO fact subset selected from the legacy generated header |
| `devices/atmega328p/timer_modes.hpp` | Concrete clock and waveform enums/tables/traits |
| `devices/atmega328p/timer_fields.hpp` | Concrete timer bit fields and register formats |
| `devices/atmega328p/timer_registers.hpp` | Access-policy-bound timer register aliases |
| `devices/atmega328p/timer_access.hpp` | Byte sequencing, caller barriers and interrupt-flag clear semantics |
| `devices/atmega328p/gpio_fields.hpp` | B/C/D GPIO register fields and the 23 real pin identities |
| `devices/atmega328p/timers.hpp` | Timer0/1/2 definitions, output routes and timer inventory |

All headers live under `grevir/avr/`; `<GrevirAVR.h>` is the generic public entry point.
Include `<grevir/avr/devices/atmega328p/timers.hpp>` explicitly for that device.
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
waveform-specific hardware count adjustments remain deferred; configuration
currently preserves this inherited model.
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

## Timer waveform modes

The matching waveform groups in the two legacy timer headers are consolidated
in `timer/mode.hpp`. `TimerMode`, `TimerPwmMode` and `TimerTop` describe normal,
CTC and PWM operation, PWM counting behavior, and fixed/OCRA/ICR TOP sources.
`TimerPwmModePhaseCorrect` recognizes both phase-correct variants.

`WaveformGeneratorMode<Enum, Code, Mode, PwmMode, TopSource, BuiltInTop>` carries
metadata. `WaveformGeneratorModes<...>::type<Mode, PwmMode, TopSource>` returns all
matching entries in declaration order; `found` reports whether that tuple is
nonempty. `built_in_type<Mode, PwmMode, Count>` returns the first exact built-in
TOP match, or `void`, including on an empty table. These are selection queries;
the later configuration layer must diagnose an unsatisfied mandatory request.

`getParamFor<Parameter>(code)` returns `setl::Optional` metadata. Unknown codes
return an empty result. Its out-parameter overload leaves the supplied value
unchanged on a miss; plain-value outputs can also be used in constant expressions.
Runtime lookup retains the first-match behavior of the legacy table. Callers are
responsible for valid device metadata; this is not a hardware mode-table validator.

Register-supplied TOP entries default to the explicitly 32-bit metadata marker
`UnspecifiedTimerTop`. It replaces the old dependency on host/target-sized `NA`;
inspect `timer_top` before treating `built_in_top` as an actual count. The helper
performs no frequency arithmetic and introduces no floating-point or 64-bit work.

ATmega328P production enums/tables now live in the explicit device binding.
The earlier host fixtures remain independent regression data for the reusable layer.

## Timer definitions and TOP access

`TimerDefinition` composes caller-provided register/field types, output-compare
metadata, capture capabilities and TOP sources. `TimerCapture<void, void, void>`
represents absent capture; `TimerInputNoiseCanceller<void>` supplies empty setting
appliers. Present capture provides edge/noise setting appliers and adds ICR to
the timer's TOP sources. Output-compare entries retain the A/B tuple ordering.
These primitives neither allocate timers nor register interrupt handlers.

The optional final `ModeTraits` argument supplies `Modes`; omitted, it uses the
legacy `WgmEnumTraits<WaveformEnum>` specialization. No concrete MCU inventory is
required by the reusable definition. `TimerDefTopRegister<ocra/icr>` identifies
the writable field; built-in TOP has no writable-register specialization.

`get_timer_top(source)` returns `setl::Optional<uint32_t>`. OCRA/ICR queries read
only that source at its field's native width. A built-in query reads the waveform
field and checks its metadata; the overload accepting a waveform enum avoids that
read. For register sources the supplied waveform is ignored, preserving legacy
behavior. Unsupported sources and empty source lists return an empty optional
without I/O. A genuine register value of zero remains present.

Built-in queries now return empty for unknown/reserved waveform codes and for
waveforms whose TOP comes from OCRA/ICR. The old code discarded the metadata's
presence flag and could report a present zero or marker. The lower-level
`TopGetter<Definition, built_in>::get` consequently returns an optional too;
OCRA/ICR getters retain native-width integer returns. Explicit result dispatch
also fixes the old compile failures for OCRA-only and empty TOP-source lists.
This does not change Base's general optional or value-tuple algorithms.

Access policies still own synchronization and hardware read/write semantics.
No register-read atomicity, physical capture behavior or noise filtering is
established by the host fixture. No floating-point or 64-bit arithmetic is added.

## Timer configuration

`TimerSettings<pwm, PwmMode, Top>` selects programmable OCRA/ICR TOP;
`TimerBuiltInSettings<pwm, PwmMode, Bits>` selects fixed TOP. Compose either with
`TimerConfiguration<Definition, Frequency, Clock, Settings, ClockTraits>`.
The final traits argument is optional and defaults to `TccrEnumTraits<ClockEnum>`;
waveform traits come from the definition. Legacy primitive names
`TimerPwmConfigutation` and `TimerPwmBuiltinTopConfigutation` retain their spelling.

Compile-time requests must have a compatible mode, supported divider and valid
count/resolution. Programmable modes must match exactly one table entry. Divider
selection uses the smaller logical capacity of counter and TOP fields, preventing
narrowing into a smaller TOP register. Built-in resolution must fit the counter
and OCRA field. Count fields are unsigned scalars with contiguous logical bits.
`actual_divider` now reports the selected prescaler, correcting an inherited
assignment of the base clock frequency.

`setupTimer()` applies the computed settings. `setFrequency(value)` returns the
TOP count, or zero for invalid/unrepresentable requests without any register I/O.
Programmable TOP below three in fast PWM (two in the retained dual-slope path)
is rejected. The low-level programmable
configuration applies clock/mode only; `TimerConfiguration` additionally writes
TOP after validation, at the field's native width. Built-in configuration writes
only clock/mode. Integer calls retain integer arithmetic; floating input/results
remain explicit choices. No implicit floating or runtime 64-bit intermediates were
introduced. Clock/mode then TOP ordering is retained; live changes are not atomic
or demonstrated glitch-free, and hardware buffering/synchronization is not modeled.

`getFrequency<Result>()` uses live register settings. The primitive overload can
accept an explicit TOP and clock. Unknown mode or unavailable TOP returns the
legacy `static_cast<Result>(-1)` sentinel (maximum for unsigned results); stopped
or unmapped clocks return zero. Optional metadata is checked before use. These
PWM calculations now use TOP+1 ticks for fast PWM and 2*TOP for dual-slope
PWM, through `timer/pwm_clock.hpp`. The legacy count helpers remain separate.
For example, 16 MHz /1 at 1 kHz fast PWM programs TOP=15999; fixed TOP=255
at /64 reports 976 Hz as an integer (976.5625 Hz with an explicit double result).
These formulas and mock effects are checked; physical PWM timing remains unvalidated.
The `OutputPin` alias is implemented by `timer/output.hpp`, included by the aggregate header.

## Timer output application

`TimerOutputPinSettings<Channel, Invert>` selects an output compare channel and
its legacy polarity. `Config::OutputPin<Settings>` exposes that output;
`TimerPwmPinConfiguration<Config, Settings...>` composes outputs with timer setup
and frequency updates. Outputs can be selected by argument index or OCR channel.
Its `pwmWrite(Settings{}, value)` now routes to the selected output with the live
TOP, correcting the inherited call to a nonexistent configuration method.
`Timer<Definition, ClockTraits>` exposes the legacy configuration aliases and a
checked `getTopCount()` (zero for unavailable metadata/TOP). Explicit clock traits
are optional; mode traits and register policies remain caller-provided.

Output compare encodings come from `COM8::type`, which must supply `clear`, `set`
and `disconnect`. GPIO comes from `OutputCompare::GpioDef`. No device enum is
hardcoded. Legacy polarity is retained: `invert_output=true` selects clear-on-compare,
low at zero and high at full duty; `false` selects set-on-compare and the opposite
endpoint levels. This documents the inherited naming rather than reversing outputs.

Count fields have at most 16 logical bits for this AVR output implementation.
They must fit every TOP the chosen configuration can generate. Repeated channels,
repeated GPIO types, and using OCRA for both TOP and duty are compile-time errors.
This is local composition validation, not a complete physical-resource inventory:
callers must provide consistent identities for aliases of the same hardware pin.
The generic cross-MCU APIs are unaffected by the AVR count bound.

`pwmWrite(value, top)` clamps finite values outside the duty interval to GPIO
endpoints. Zero/negative/unrepresentable TOP and nonfinite values cause no IO.
`pwmWritef(float, top)` explicitly selects fractional arithmetic and checks bounds
before conversion, fixing the inherited ill-formed narrowing initializer. Duty
counts truncate; a result quantized to zero becomes the GPIO endpoint. The raw
`pwmWriteAbsoluteValue` only checks field capacity and does not manage COM/GPIO.

`pwmAdjust(old, new)` now uses floor(old_compare * new / old), with a 32-bit
product of factors bounded by 65535. It widens before multiplication, avoiding
16-bit promotion overflow without floating or 64-bit work. A stale compare above
old TOP saturates to the full-duty endpoint. Unchanged/invalid TOP or a COM mode
other than this output's configured PWM mode causes no adjustment. The composed
frequency setter skips adjustment on rejection; it can read old TOP, but writes
nothing for an invalid request. Fixed TOP changes leave duty counts unchanged.

Compare values are written before connecting PWM, endpoint latches before
disconnecting PWM, and initial output settings before enabling DDR. Host traces
verify this order and preservation of other channels. They do not establish
atomic updates, waveform timing, glitch-free transitions, timer buffering or
interrupt synchronization. Actual target access policies still own those details.

## ATmega328P timer and GPIO bindings

Include `grevir/avr/devices/atmega328p/timers.hpp` and instantiate
`ardo::sys::avr::arch_atmega328p::TimerBindings<ByteAccess, Barrier>`.
`Timer0`, `Timer1`, `Timer2` and `Timers` expose the three timer interfaces;
`Gpio` contains the physical port identities such as `ppPB1` and `ppPD6`.
No access/barrier defaults, CPU clock, board reservation or frequency preset is
injected. Supply those choices explicitly. `GpioBindings<ByteAccess, Barrier>`
is also independently usable. PC7 is not invented to fill an eight-bit port.

The legacy clock, mode, COM and register-field groups match between both source
headers. They now have one device implementation split by responsibility. Raw
facts remain under `generated/`; this is a selected timer/GPIO subset, not a full
regeneration. All 276 selected bit/address facts match the independently retrieved
[avr-libc ATmega328P definitions](https://github.com/avrdudes/avr-libc/blob/main/include/avr/iom328p.h).
The full raw-device assignment remains pending in the migration ledger.

Timer0 outputs bind to PD6/PD5, Timer1 to PB1/PB2, and Timer2 to PB3/PD3.
Timer1 supplies ICR/capture; Timer0/2 do not. Definitions include native TCCRnB
aliases so their force-compare fields can be selected. Timer1 capture interrupt
fields and Timer2's counter-update status are included, correcting omissions in
the old field inventories. Old board-specific demonstration aliases are replaced
by the reusable configuration aliases, so Timer2 no longer embeds a preset that
claims OCRA as both TOP and an output.

The timer adapter calls `ByteAccess` with eight-bit operations and final memory
addresses. Synthetic TCCRnA/B fields are assembled explicitly, independent of host
endianness; masked changes touch only affected bytes. Timer1 words read low/high
and write high/low. Word operations and masked timer updates use the supplied
RAII barrier. Timer interrupt flags use write-one-to-clear commands without first
reading and echoing unrelated flags. These choices follow the timer-register and
16-bit access sections of the [Microchip ATmega328P datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-7810-automotive-microcontrollers-atmega328p_datasheet.pdf)
(sections 14.9.7, 15.3, 15.11.9 and 17.11.7 in 7810D). The caller must supply a
working, nestable interrupt-state-preserving barrier for target use.

Host fixtures model Timer1's shared high-byte latch, flag clearing and force-compare
strobes, and assert register effects and pin routes. They do not model PWM waveforms,
interrupt execution, electrical pins or Timer2 asynchronous synchronization. Current
configuration coverage assumes synchronous timer clocks, enabled peripherals and
caller-managed ownership. Waveform-specific TOP conversion is implemented and
checked on the host; the installed portable adapter generates fast-PWM
candidates from all three declarations and applies them through these bindings. No target compiler or hardware validation was run.

## Validation and use

Apple Clang 21 / arm64 macOS / C++23 checks:

- Twenty-one public headers compile independently, alongside address/type assertions and
  a compile-only volatile-access user.
- Fifty host cases pass. They cover
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
- Four waveform cases verify all 21 legacy fixture entries, absent metadata for
  reserved/unknown encodings, preserved outputs on misses, and selected encodings
  applied to split mock register fields. Twenty-eight static assertions cover
  legacy getters, exact TOP selection, unsupported requests, empty tables,
  arbitrary encodings and phase-correct classification. Native compilation first
  reproduced missing `found` on nonempty tables and missing `built_in_type` on
  empty tables; both queries are now consistent.
- Five definition cases verify control-bit preservation, absent-capability no-ops,
  native-width TOP reads, invalid built-in requests and restricted/empty inventories.
  Thirteen static assertions and compile-only reads cover capabilities, field types,
  source composition, result types and explicit traits without executing MMIO.
- Six configuration cases check ICR/OCRA and built-in setup, preserved fields,
  native write widths, rejected updates without IO, narrower TOPs and checked live
  frequency reads. Twelve static assertions check divider/count/capacity results.
  A standalone valid/seven-rejection probe checks invalid frequencies, counts,
  resolution and missing modes. Dynamic integer configuration host IR contains no
  floating or 64-bit arithmetic; this does not establish target instruction cost.
- Eight output cases cover endpoint polarity, write order, fractional conversion,
  integer rescaling boundaries, disconnected channels, frequency rejection, checked
  TOP reads, 8-/16-bit access and OCRA-TOP with OCRB output. Ten static assertions
  and compile-only MMIO uses cover aliases and composition. One valid/seven rejected
  standalone probes check duplicate channels/GPIO, TOP conflicts, missing configured
  outputs and count capacity. Native UBSan/float-cast-overflow checks pass; dynamic
  integer duty/adjustment IR has no floating or 64-bit arithmetic.
- Seven concrete-device cases and nineteen static assertions check three timers,
  physical pin routes, actual byte addresses, word sequencing/barriers, W1C flags,
  native force-compare access and capture/status fields. One valid/three rejected
  native probes check unavailable capture/modes and OCRA TOP/output conflicts.
  The installed consumer also configures and updates the concrete Timer1 binding.
- Isolated production/host builds and an installed consumer pass. The production
  consumer has Catch2 and Test Support discovery disabled; it supplies its own
  memory policy and exercises explicit clock/mode traits, timer TOP access, configuration and output composition with custom COM encodings.
  Test fixtures come from `grevir::test_support` only in host tests.

With Base, Core, Registers and their dependencies installed:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=<prefix> -DGREVIR_BUILD_COMPILE_CHECKS=ON
cmake --build build
cmake --install build --prefix <prefix>
```

Consumers use `find_package(grevir-avr CONFIG REQUIRED)` and link `grevir::avr`.
Host tests opt in with `GREVIR_BUILD_HOST_TESTS=ON` and installed Test Support/Catch2.
Arduino metadata is present; no Arduino sketch or target compilation is claimed.
Other device/peripheral inventories, a target AVR barrier implementation and
board mappings remain later increments. The installed Timer0/1/2 fast-PWM adapter
is under `devices/atmega328p/pwm_backend.hpp`; the experiment now forwards to it.
Other timer features remain TBD. The
original Ardoinus checkout remains unchanged.

## Installed portable PWM integration

The fixed-frequency ATmega328P PWM MVP now uses installed Core, Peripherals and AVR
headers. Core collects module requests and existing resource claims; AVR supplies
candidates and typed endpoints. Application setup initializes the selected owners
before parameter/module callbacks. See the workspace's
`dev-docs/GrevirPwmIntegration.md` for the complete example, resource identity rules,
startup preconditions and current limits. AVR compiler/hardware validation remains
on hold; native package installation does not establish MCU toolchain support.
