#include <GrevirAVR.h>
#include <array>
#include <cstring>

namespace {
namespace avr = ardo::sys::avr;
struct Memory {
  static inline std::array<unsigned char, 64> bytes{};
  static inline std::array<std::ptrdiff_t, 8> writes{};
  static inline unsigned count = 0;
  template <typename T>
  static T read(std::ptrdiff_t address) {
    T value;
    std::memcpy(&value, bytes.data() + address, sizeof(value));
    return value;
  }
  template <typename T>
  static void write(std::ptrdiff_t address, T value) {
    writes[count++] = address;
    std::memcpy(bytes.data() + address, &value, sizeof(value));
  }
  template <typename T>
  static void modify(std::ptrdiff_t address, T value, T mask) {
    write<T>(address, static_cast<T>((read<T>(address) & ~mask) | (value & mask)));
  }
};
struct Barrier {
  static inline unsigned enters = 0;
  static inline unsigned exits = 0;
  Barrier() { ++enters; }
  ~Barrier() { ++exits; }
};
using Binding = avr::base::RegisterSelector<Memory>;
using Bit = setl::BitsRW<bool, 7>;
template <std::ptrdiff_t Address>
using Reg = avr::base::Register<setl::BitFields<Bit>,
  avr::nfp::IoRegisterDef<std::uint8_t, Address, 0x20>, Binding::IoAccessor>;
using Definition = avr::base::GpioPortDefinition<Bit, Bit, Bit, Reg<5>, Reg<3>, Reg<4>>;
struct Pin : avr::base::GpioPort<Pin, Definition, Barrier> {};
enum class ClockCode { stopped, unit, eight };
struct ClockTraits {
  using FreqMapping = avr::base::DividerMappings<
    avr::base::DividerMapping<ClockCode, ClockCode::unit, 1>,
    avr::base::DividerMapping<ClockCode, ClockCode::eight, 8>>;
  static constexpr ClockCode null_value = ClockCode::stopped;
};
constexpr auto selected = avr::base::getClockDivider<ClockCode, ClockTraits>(1, 256, 8, false);
static_assert(selected == ClockCode::eight);
constexpr auto top = avr::base::getClockTimerTop<ClockCode, ClockTraits>(selected, 1, 256, false);
static_assert(top == 32);

enum class WaveCode : unsigned char { fixed = 31, capture = 6 };
using FixedMode = avr::base::WaveformGeneratorMode<WaveCode, WaveCode::fixed,
  avr::base::TimerMode::pwm, avr::base::TimerPwmMode::fast, avr::base::TimerTop::built_in, 255>;
using CaptureMode = avr::base::WaveformGeneratorMode<WaveCode, WaveCode::capture,
  avr::base::TimerMode::pwm, avr::base::TimerPwmMode::phase_correct, avr::base::TimerTop::icr>;
using WaveModes = avr::base::WaveformGeneratorModes<FixedMode, CaptureMode>;
static_assert(WaveModes::found<avr::base::TimerMode::pwm, avr::base::TimerPwmMode::fast,
  avr::base::TimerTop::built_in>);
static_assert(std::is_same_v<WaveModes::built_in_type<avr::base::TimerMode::pwm,
  avr::base::TimerPwmMode::fast, 255>, FixedMode>);
}

int main() {
  Pin::configure(true, true);
  if (Memory::bytes[0x25] != 0x80 || Memory::bytes[0x24] != 0x80
      || Memory::writes[0] != 0x25 || Memory::writes[1] != 0x24) {
    return 1;
  }
  avr::base::BidirectionalGpioPort<Pin>::set(true);
  if (Memory::bytes[0x24] != 0 || Memory::bytes[0x25] != 0x80
      || Memory::writes[2] != 0x24 || Memory::writes[3] != 0x25) {
    return 2;
  }
  Memory::bytes[0x23] = 0x80;
  if (!Pin::get() || Barrier::enters != 2 || Barrier::exits != 2
      || Memory::bytes[5] != 0 || Memory::bytes[0x05 + 0x20 + 1] != 0) {
    return 3;
  }
  if (avr::base::getTimerFrequency<double, ClockCode, ClockTraits>(top, selected, 256, false) != 1) {
    return 4;
  }
  const auto source = WaveModes::getParamFor<avr::base::WaveformGeneratorModeParam::timer_top>(WaveCode::capture);
  if (!source.is_present() || source.get() != avr::base::TimerTop::icr) {
    return 5;
  }
  if (WaveModes::getParamFor<avr::base::WaveformGeneratorModeParam::timer_top>(static_cast<WaveCode>(255)).is_present()) {
    return 6;
  }
  return 0;
}
