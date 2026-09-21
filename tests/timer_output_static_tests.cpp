#include "timer_output_fixture.hpp"
using namespace timer_output_fixture;
using Hardware = Model<VolatileAccess>;
using A = Hardware::Config::OutputPin<SettingsA>;
using B = Hardware::Config::OutputPin<SettingsB>;
using Pins = TimerPwmPinConfiguration<Hardware::Config, SettingsA, SettingsB>;
static_assert(A::COM_MODE == EnumCOMn::clear);
static_assert(B::COM_MODE == EnumCOMn::set);
static_assert(std::is_same_v<Pins::PwmPinByArgN<0>, A>);
static_assert(std::is_same_v<Pins::PwmPinByOcrReg<OcrEnum::OcrB>, B>);
static_assert(std::is_same_v<typename A::OCR::type, std::uint16_t>);
static_assert(A::CountField::capacity == 65535);
static_assert(Model<VolatileAccess, std::uint8_t>::Config::OutputPin<SettingsA>::CountField::capacity == 255);
using Facade = Timer<Hardware::Definition>;
using Accurate = Facade::FrequencyAccurate<1000, 16000000, TimerMode::pwm, TimerPwmMode::fast, TimerTop::icr>;
static_assert(std::is_same_v<Accurate, Hardware::Config>);
using Fixed = Facade::BuiltInTop<1000, 16000000, TimerMode::pwm, TimerPwmMode::fast, 8>;
static_assert(Fixed::OutputPin<SettingsA>::max_top == 255);
static_assert(sizeof(Facade::PwmPinConfiguration<Accurate>) > 0);
// Compile-only output MMIO bindings; never called on the host.
void instantiate_timer_output() {
  Pins::setup();
  Pins::pwmWrite(SettingsB{}, 500);
  (void)Pins::setFrequency(2000);
  A::pwmWritef(0.25f, std::uint16_t{8000});
  (void)Facade::getTopCount();
}
