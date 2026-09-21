#include <grevir/avr/devices/atmega328p/timers.hpp>
namespace d = ardo::sys::avr::arch_atmega328p;
namespace b = ardo::sys::avr::base;
struct NativeOnlyBarrier {};
using Device = d::TimerBindings<b::VolatileAccess, NativeOnlyBarrier>;
static_assert(std::tuple_size_v<Device::Timers> == 3);
static_assert(d::rrTCCR0A::addr == 0x44 && d::rrTCCR0B::addr == 0x45);
static_assert(d::rrTCCR1A::addr == 0x80 && d::rrTCCR1B::addr == 0x81);
static_assert(d::rrTCCR2A::addr == 0xb0 && d::rrTCCR2B::addr == 0xb1);
static_assert(d::rrTCNT1::addr == 0x84 && d::rrICR1::addr == 0x86);
static_assert(d::rrOCR1A::addr == 0x88 && d::rrOCR1B::addr == 0x8a);
static_assert(d::rrPORTB::addr == 0x25 && d::rrPORTD::addr == 0x2b);
static_assert(std::is_same_v<Device::Timer0Def::OcrType<b::OcrEnum::OcrA>::GpioDef, Device::Gpio::ppPD6>);
static_assert(std::is_same_v<Device::Timer0Def::OcrType<b::OcrEnum::OcrB>::GpioDef, Device::Gpio::ppPD5>);
static_assert(std::is_same_v<Device::Timer1Def::OcrType<b::OcrEnum::OcrA>::GpioDef, Device::Gpio::ppPB1>);
static_assert(std::is_same_v<Device::Timer1Def::OcrType<b::OcrEnum::OcrB>::GpioDef, Device::Gpio::ppPB2>);
static_assert(std::is_same_v<Device::Timer2Def::OcrType<b::OcrEnum::OcrA>::GpioDef, Device::Gpio::ppPB3>);
static_assert(std::is_same_v<Device::Timer2Def::OcrType<b::OcrEnum::OcrB>::GpioDef, Device::Gpio::ppPD3>);
static_assert(!Device::Timer0Def::TimerCaptureType::has_input_capture && Device::Timer1Def::TimerCaptureType::has_input_capture);
static_assert(!Device::Timer2Def::TimerCaptureType::has_input_capture);
template <typename T> concept HasPC7 = requires { typename T::ppPC7; };
static_assert(!HasPC7<Device::Gpio>);
using T0 = Device::Timer0::FrequencyAccurate<2000,16000000,b::TimerMode::pwm,b::TimerPwmMode::fast,b::TimerTop::ocra>;
using T1 = Device::Timer1::FrequencyAccurate<1000,16000000,b::TimerMode::pwm,b::TimerPwmMode::fast,b::TimerTop::icr>;
using T2 = Device::Timer2::FrequencyAccurate<2000,16000000,b::TimerMode::pwm,b::TimerPwmMode::fast,b::TimerTop::ocra>;
static_assert(T0::Config::cs_value == d::EnumCS0::clk64 && T0::Config::top_count == 125);
static_assert(T1::Config::cs_value == d::EnumCS1::clk1 && T1::Config::top_count == 16000);
static_assert(T2::Config::cs_value == d::EnumCS2::clk32 && T2::Config::top_count == 250);
// Compiles concrete volatile bindings; never executed on the host.
void compile_atmega328p() {
  T0::setupTimer(); T1::setupTimer(); T2::setupTimer();
  (void)T1::setFrequency(2000);
  Device::Timer0Def::Registers::ReadModifyWrite(d::BitsFOC0A{true});
  Device::Timer2Def::Registers::ReadModifyWrite(d::BitsFOC2B{true});
}
