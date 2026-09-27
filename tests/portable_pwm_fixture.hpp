#pragma once
#include "atmega328p_fixture.hpp"
#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
#include <grevir/peripherals/timer/own.hpp>

namespace portable_pwm_fixture {
using namespace grevir::pwm;
namespace m = grevir::pwm::atmega328p;
using atmega328p_mock::Memory;
using atmega328p_mock::word;
inline bool prerequisite_ready = false;
inline bool writes_after_prerequisite = true;
struct TrackedBytes : atmega328p_mock::Bytes {
  template <typename T>
  static void write(std::ptrdiff_t address, T value) {
    writes_after_prerequisite &= prerequisite_ready;
    atmega328p_mock::Bytes::write(address, value);
  }
  template <typename T>
  static void modify(std::ptrdiff_t address, T value, T mask) {
    writes_after_prerequisite &= prerequisite_ready;
    atmega328p_mock::Bytes::modify(address, value, mask);
  }
};
using Device = ardo::sys::avr::arch_atmega328p::TimerBindings<TrackedBytes,atmega328p_mock::Barrier>;
using Backend = m::Backend<Device,16'000'000>;
inline constexpr unsigned pin_b1 = m::physical_pin<typename Device::Gpio::ppPB1>();
inline constexpr unsigned pin_b2 = m::physical_pin<typename Device::Gpio::ppPB2>();
inline constexpr unsigned pin_d6 = m::physical_pin<typename Device::Gpio::ppPD6>();
inline std::vector<unsigned> callbacks;
inline bool ready_during_params = false;
inline unsigned custom_params_setup = 0;
inline unsigned custom_params_loop = 0;
struct Prerequisite : ardo::ModuleBase<> {
  static void runSetup() { prerequisite_ready = true; callbacks.push_back(9); }
};
struct ReadyProbe {
  using Claims = ardo::ResourceClaim<>;
  static void runSetup() { ready_during_params = word(0x86) == 15999; callbacks.push_back(0); }
  static void runLoop() {}
};

using MotorTimer = grevir::timer::Own<PwmRequest<"left",
  Frequency<Hertz<1000>,WithinPpm<10'000>>, DutyStepAtMost<1,256>,
  For<Target::avr,Pin<pin_b1>,Frequency<Hertz<1000>,Exact>,avr::TopFromIcr>,
  For<Target::esp32,Pin<18>,esp32::ApbClock>>,
  PwmRequest<"right",Frequency<Hertz<1000>,Exact>, DutyStepAtMost<1,256>,
    Pin<pin_b2>,avr::TopFromIcr>,
  grevir::timer::For<Target::avr,grevir::timer::CounterBitsAtLeast<16>>,
  grevir::timer::For<Target::atmega328p,
    grevir::timer::RequireTimer<grevir::timer::atmega328p::Timer1>>>;
using MotorRequest = grevir::timer::Instance<"motor",MotorTimer>;
template <typename Plan>
struct Motor : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"left">,
  typename Plan::template Pwm<"right">,ReadyProbe>> {
  using Base = ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"left">,
    typename Plan::template Pwm<"right">,ReadyProbe>>;
  using Left = typename Plan::template Pwm<"left">;
  using Right = typename Plan::template Pwm<"right">;
  static void paramsSetup() { ++custom_params_setup; Base::paramsSetup(); }
  static void paramsLoop() { ++custom_params_loop; Base::paramsLoop(); }
  static void runSetup() { callbacks.push_back(1); Left::write(1,4); Right::write(3,4); }
  static void runLoop() { callbacks.push_back(3); }
};
template <typename Plan>
struct Fan : ardo::ModuleBase<ardo::Parameters<>> {
  static void runSetup() { callbacks.push_back(2); }
  static void runLoop() { callbacks.push_back(4); }
};
using MotorModule = grevir::RequestedModule<setl::TypeArgs<MotorRequest>,Motor,
  ardo::ResourceClaim<>,grevir::ExistingModule<Prerequisite>>;
using FanModule = grevir::RequestedModule<setl::TypeArgs<>,Fan,ardo::ResourceClaim<>,MotorModule>;
using App = grevir::AllocatedApplication<Backend,FanModule,MotorModule>;
using Reverse = grevir::AllocatedApplication<Backend,MotorModule,FanModule>;
static_assert(App::plan.requests == Reverse::plan.requests);
static_assert(App::plan.candidates == Reverse::plan.candidates);
static_assert(App::SelectedClaims::has_resource<ardo::HardwareTimer<1>>::value);
static_assert(App::SelectedClaims::has_resource<ardo::GPIOResource<pin_b1>>::value);
static_assert(!App::SelectedClaims::has_resource<ardo::HardwareTimer<0>>::value);
static_assert(App::OwnerClaims<MotorRequest::name>
  ::has_resource<ardo::HardwareTimer<1>>::value);

using AuxiliaryTimer = grevir::timer::Own<PwmRequest<"pwm",
  Frequency<Hertz<15625,16>,Exact>,DutyStepAtMost<1,256>,Pin<pin_d6>,
  avr::BuiltInTop>,
  grevir::timer::For<Target::atmega328p,
    grevir::timer::RequireTimer<grevir::timer::atmega328p::Timer0>>>;
using AuxiliaryRequest = grevir::timer::Instance<"auxiliary",AuxiliaryTimer>;
template <typename Plan>
struct Auxiliary : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"pwm">>> {};
using AuxiliaryModule = grevir::RequestedModule<setl::TypeArgs<AuxiliaryRequest>,Auxiliary>;
using TwoOwnerApp = grevir::AllocatedApplication<Backend,MotorModule,AuxiliaryModule>;
static_assert(TwoOwnerApp::plan.ok());
static_assert(TwoOwnerApp::OwnerClaims<AuxiliaryRequest::name>
  ::has_resource<ardo::HardwareTimer<0>>::value);
static_assert(!TwoOwnerApp::OwnerClaims<AuxiliaryRequest::name>
  ::has_resource<ardo::HardwareTimer<1>>::value);

struct Timer1Claim {
  using Claims = ardo::ResourceClaim<ardo::range_claim<ardo::HardwareTimer<1>,0,1>>;
  static void runSetup() {}
  static void runLoop() {}
};
struct LegacyOwner : ardo::ModuleBase<ardo::Parameters<Timer1Claim>> {};
struct Dependent : ardo::ModuleBase<ardo::Parameters<>,ardo::DependentModules<LegacyOwner>> {};
using Legacy = grevir::ExistingModule<Dependent>;
using Blocked = Backend::Allocate<setl::TypeArgs<MotorRequest>,Legacy::Claims::Resources>;
static_assert(Blocked::plan.diagnostic.status == Status::reserved);
using PinBlocked = Backend::Allocate<setl::TypeArgs<MotorRequest>,setl::TypeArgs<ardo::GPIOResource<pin_b1>>>;
static_assert(PinBlocked::plan.diagnostic.status == Status::reserved);
} // namespace portable_pwm_fixture
