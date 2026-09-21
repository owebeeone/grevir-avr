#include <grevir/avr/timer/definition.hpp>
#include <grevir/avr/register.hpp>
#include <grevir/avr/register_definitions.hpp>

namespace {
using namespace ardo::sys::avr::base;
enum class Mode : unsigned char { fixed, compare };
using ModeBits = setl::BitsRW<Mode, 1, 0>;
struct ExplicitTraits {
  using Modes = WaveformGeneratorModes<
    WaveformGeneratorMode<Mode, Mode::fixed, TimerMode::normal, TimerPwmMode::none, TimerTop::built_in, 255>,
    WaveformGeneratorMode<Mode, Mode::compare, TimerMode::ctc, TimerPwmMode::none, TimerTop::ocra>>;
};
using CompareBits = setl::BitsRW<std::uint8_t>;
using CaptureBits = setl::BitsRW<std::uint16_t>;
using NoiseBits = setl::BitsRW<bool, 7>;
using EdgeBits = setl::BitsRW<bool, 6>;
using Compare = OutputCompare<CompareBits, void, void, void, void, void>;
using Capture = TimerCapture<CaptureBits, NoiseBits, EdgeBits>;
using NoCapture = TimerCapture<void, void, void>;
using Access = RegisterSelector<VolatileAccess>;
template <std::ptrdiff_t Address, typename T, typename Bits>
using Reg = Register<setl::BitFields<Bits>, ardo::sys::avr::nfp::MemRegisterDef<T, Address>, Access::IoAccessor>;
using Registers = std::tuple<Reg<1, std::uint8_t, ModeBits>, Reg<2, std::uint8_t, CompareBits>,
  Reg<4, std::uint16_t, CaptureBits>>;
using Sources = setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra>;
using Definition = TimerDefinition<ModeBits, void, void, void, CompareBits,
  std::tuple<Compare>, Capture, Registers, Sources, ExplicitTraits>;
using Simple = TimerDefinition<ModeBits, void, void, void, CompareBits,
  std::tuple<Compare>, NoCapture, Registers, Sources, ExplicitTraits>;
static_assert(Capture::has_input_capture && Capture::has_capture_register && Capture::has_edge_selection);
static_assert(Capture::NoiseCanceller::has_noise_canceller);
static_assert(!NoCapture::has_input_capture && !NoCapture::has_capture_register && !NoCapture::has_edge_selection);
static_assert(!NoCapture::NoiseCanceller::has_noise_canceller);
static_assert(std::is_same_v<Definition::OcrType<OcrEnum::OcrA>, Compare>);
static_assert(std::is_same_v<Definition::TimerDefTopRegister<TimerTop::ocra>, CompareBits>);
static_assert(std::is_same_v<Definition::TimerDefTopRegister<TimerTop::icr>, CaptureBits>);
static_assert(std::is_same_v<Definition::TimerTopTuple,
  setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra, TimerTop::icr>>);
static_assert(std::is_same_v<Simple::TimerTopTuple, Sources>);
static_assert(std::is_same_v<decltype(Definition::get_timer_top(TimerTop::ocra)), setl::Optional<std::uint32_t>>);
static_assert(std::is_same_v<decltype(TopGetter<Definition, TimerTop::ocra>::get()), std::uint8_t>);
static_assert(std::is_same_v<decltype(TopGetter<Definition, TimerTop::icr>::get()), std::uint16_t>);
static_assert(std::is_same_v<decltype(TopGetter<Definition, TimerTop::built_in>::get()), setl::Optional<std::uint32_t>>);

// Instantiate the actual read paths; never execute volatile MMIO in host checks.
[[maybe_unused]] setl::Optional<std::uint32_t> compileReads(TimerTop top, Mode mode) {
  return top == TimerTop::built_in ? Definition::get_timer_top(top, mode) : Definition::get_timer_top(top);
}
} // namespace
