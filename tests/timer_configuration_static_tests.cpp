#include "timer_configuration_fixture.hpp"
using namespace timer_configuration_fixture;
using TimerModel = Model<VolatileAccess>;
using Capture = TimerConfiguration<TimerModel::Definition, 1000, 16000000, CaptureSettings>;
static_assert(Capture::Config::cs_value == EnumCS1::clk1);
static_assert(Capture::Config::top_count == 15999);
using Phase = TimerConfiguration<TimerModel::Definition, 1000, 16000000, CompareSettings>;
static_assert(Phase::Config::top_count == 8000);
using Fixed = TimerConfiguration<TimerModel::Definition, 1000, 16000000, FixedSettings>;
static_assert(Fixed::Config::actual_divider == 64);
static_assert(Fixed::Config::top_count == 255);
using Narrow = Model<VolatileAccess, std::uint16_t, std::uint8_t>;
using NarrowConfig = TimerConfiguration<Narrow::Definition, 1000, 16000000, CaptureSettings>;
static_assert(NarrowConfig::Config::top_resolution == 8);
static_assert(NarrowConfig::Config::cs_value == EnumCS1::clk64);
static_assert(NarrowConfig::Config::top_count == 249);
using Partial = setl::BitsRW<std::uint16_t, 15, 14, 13, 12>;
static_assert(ardo::sys::avr::base::nfp::TimerCountField<Partial>::capacity == 15);
static_assert(ardo::sys::avr::base::nfp::TimerCountField<Partial>::width == 4);
static_assert(ardo::sys::avr::base::nfp::TimerCountField<setl::BitsRW<std::uint32_t>>::width == 32);

// Hardware period boundaries: fast PWM includes both zero and TOP.
using Boundary = TimerConfiguration<TimerModel::Definition, 62500, 16000000, FixedSettings>;
static_assert(Boundary::Config::actual_divider == 1);
static_assert(getPwmTop<EnumCS1>(EnumCS1::clk1, 1000, 16000000, false) == 15999);
static_assert(getPwmTop<EnumCS1>(EnumCS1::clk1, 1000, 16000000, true) == 8000);
static_assert(getPwmFrequency<std::uint32_t,EnumCS1>(255,EnumCS1::clk1,16000000,false) == 62500);
static_assert(getPwmFrequency<std::uint32_t,EnumCS1>(65535,EnumCS1::clk1,65536,false) == 1);
static_assert(getPwmFrequency<std::uint32_t,EnumCS1>(0xffffffffu,EnumCS1::clk1,16000000,false) == 0);
