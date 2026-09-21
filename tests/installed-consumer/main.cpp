#include <GrevirAVR.h>
#include <grevir/avr/devices/atmega328p/timers.hpp>
#include <array>
#include <cstring>

namespace {
namespace avr = ardo::sys::avr;
struct Memory {
  static inline std::array<unsigned char, 256> bytes{};
  static inline std::array<std::ptrdiff_t, 64> writes{};
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

struct WaveTraits { using Modes = WaveModes; };
using WaveBits = setl::BitsRW<WaveCode, 4, 3, 2, 1, 0>;
using ClockBits = setl::BitsRW<ClockCode, 7, 6, 5>;
using CompareBits = setl::BitsRW<std::uint8_t>;
using WaveRegister = avr::base::Register<setl::BitFields<WaveBits, ClockBits>,
  avr::nfp::MemRegisterDef<std::uint8_t, 10>, Binding::IoAccessor>;
using CompareRegister = avr::base::Register<setl::BitFields<CompareBits>,
  avr::nfp::MemRegisterDef<std::uint8_t, 11>, Binding::IoAccessor>;
// Explicit output encodings deliberately differ from the classic AVR fixture.
enum class OutputCode : unsigned char { clear = 1, disconnect = 2, set = 3 };
using OutputBits = setl::BitsRW<OutputCode, 6, 5>;
using OutputRegister = avr::base::Register<setl::BitFields<OutputBits>,
  avr::nfp::MemRegisterDef<std::uint8_t, 12>, Binding::IoAccessor>;
using Compare = avr::base::OutputCompare<CompareBits, void, void, OutputBits, OutputBits, Pin>;
using TimerDefinition = avr::base::TimerDefinition<WaveBits, ClockBits, void, void, CompareBits,
  std::tuple<Compare>, avr::base::TimerCapture<void, void, void>,
  std::tuple<WaveRegister, CompareRegister, OutputRegister>,
  setl::ValueTuple<avr::base::TimerTop, avr::base::TimerTop::built_in, avr::base::TimerTop::ocra>, WaveTraits>;
using FixedSettings = avr::base::TimerBuiltInSettings<avr::base::TimerMode::pwm,
  avr::base::TimerPwmMode::fast, 8>;
using Configuration = avr::base::TimerConfiguration<TimerDefinition, 1, 256, FixedSettings, ClockTraits>;
static_assert(Configuration::Config::actual_divider == 1);
using Timer = avr::base::Timer<TimerDefinition, ClockTraits>;
using FacadeConfiguration = Timer::BuiltInTop<1, 256, avr::base::TimerMode::pwm,
  avr::base::TimerPwmMode::fast, 8>;
static_assert(std::is_same_v<Configuration, FacadeConfiguration>);
using OutputSettings = avr::base::TimerOutputPinSettings<avr::base::OcrEnum::OcrA, true>;
using Outputs = Timer::PwmPinConfiguration<Configuration, OutputSettings>;
using Device = avr::arch_atmega328p::TimerBindings<Memory, Barrier>;
using DeviceConfig = Device::Timer1::FrequencyAccurate<1000, 16000000,
  avr::base::TimerMode::pwm, avr::base::TimerPwmMode::fast, avr::base::TimerTop::icr>;
using DeviceOutputs = Device::Timer1::PwmPinConfiguration<DeviceConfig, OutputSettings>;
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
  Memory::write<std::uint8_t>(10, 31);
  Memory::write<std::uint8_t>(11, 201);
  const auto fixed_top = TimerDefinition::get_timer_top(avr::base::TimerTop::built_in);
  const auto compare_top = TimerDefinition::get_timer_top(avr::base::TimerTop::ocra);
  if (!fixed_top.is_present() || fixed_top.get() != 255
      || !compare_top.is_present() || compare_top.get() != 201) {
    return 7;
  }
  if (TimerDefinition::get_timer_top(avr::base::TimerTop::icr).is_present()
      || TimerDefinition::get_timer_top(avr::base::TimerTop::built_in, WaveCode::capture).is_present()) {
    return 8;
  }
  Configuration::setupTimer();
  if (Memory::bytes[10] != (31 | (1 << 5)) || Configuration::get_top_count() != 255
      || Configuration::getFrequency<double>() != 1.0) {
    return 9;
  }
  const auto before = Memory::count;
  if (Configuration::setFrequency(0) != 0 || Memory::count != before) {
    return 10;
  }
  Outputs::setup();
  Outputs::pwmWrite(OutputSettings{}, 64);
  if (Memory::bytes[11] != 64 || Memory::bytes[12] != (1 << 5) || Timer::getTopCount() != 255) {
    return 11;
  }
  Outputs::pwmWrite(OutputSettings{}, 0);
  if (Memory::bytes[12] != (2 << 5) || Memory::bytes[0x25] != 0) {
    return 12;
  }
  DeviceOutputs::setup();
  DeviceOutputs::pwmWrite(OutputSettings{}, 4000);
  if (DeviceOutputs::setFrequency(2000) != 7999 || Device::Timer1::getTopCount() != 7999
      || Memory::bytes[0x86] != 0x3f || Memory::bytes[0x87] != 0x1f
      || Memory::bytes[0x88] != 0xcf || Memory::bytes[0x89] != 0x07
      || (Memory::bytes[0x24] & 2) == 0) {
    return 13;
  }
  return 0;
}
