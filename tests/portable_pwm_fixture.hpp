#pragma once
#include "atmega328p_fixture.hpp"
#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>

namespace portable_pwm_fixture {
using namespace grevir::pwm;
namespace m = grevir::pwm::atmega328p;
using atmega328p_mock::Memory;
using atmega328p_mock::word;
using Device = atmega328p_mock::Bindings;
using Backend = m::Backend<Device,16'000'000>;
inline std::vector<unsigned> callbacks;
inline bool ready_during_params = false;
struct ReadyProbe {
  using Claims = ardo::ResourceClaim<>;
  static void runSetup() { ready_during_params = word(0x86) == 15999; callbacks.push_back(0); }
  static void runLoop() {}
};

using MotorRequest = Instance<"motor",PwmRequest<"pwm",
  Frequency<Hertz<1000>,WithinPpm<10'000>>, DutyStepAtMost<1,256>,
  For<Target::avr,Pin<m::PB1>,Frequency<Hertz<1000>,Exact>,avr::TopFromIcr>,
  For<Target::esp32,Pin<18>,esp32::ApbClock>>,1>;
using FanRequest = Instance<"fan",PwmRequest<"pwm",
  Frequency<Hertz<1000>,Exact>, DutyStepAtMost<1,256>, Pin<m::PB2>,avr::TopFromIcr>,1>;
template <typename Plan>
struct Motor : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"motor">,ReadyProbe>> {
  using Pwm = typename Plan::template Pwm<"motor">;
  static void runSetup() { callbacks.push_back(1); Pwm::write(1,4); }
  static void runLoop() { callbacks.push_back(3); }
};
template <typename Plan>
struct Fan : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"fan">>> {
  using Pwm = typename Plan::template Pwm<"fan">;
  static void runSetup() { callbacks.push_back(2); Pwm::write(3,4); }
  static void runLoop() { callbacks.push_back(4); }
};
using MotorModule = grevir::RequestedModule<setl::TypeArgs<MotorRequest>,Motor>;
using FanModule = grevir::RequestedModule<setl::TypeArgs<FanRequest>,Fan,ardo::ResourceClaim<>,MotorModule>;
using App = grevir::AllocatedApplication<Backend,FanModule,MotorModule>;
using Reverse = grevir::AllocatedApplication<Backend,MotorModule,FanModule>;
static_assert(App::Allocation::plan.requests == Reverse::Allocation::plan.requests);
static_assert(App::Allocation::plan.candidates == Reverse::Allocation::plan.candidates);
static_assert(App::Allocation::Claims::has_resource<ardo::HardwareTimer<1>>::value);
static_assert(App::Allocation::Claims::has_resource<ardo::GPIOResource<m::PB1>>::value);
static_assert(!App::Allocation::Claims::has_resource<ardo::HardwareTimer<0>>::value);

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
using PinBlocked = Backend::Allocate<setl::TypeArgs<MotorRequest>,setl::TypeArgs<ardo::GPIOResource<m::PB1>>>;
static_assert(PinBlocked::plan.diagnostic.status == Status::reserved);
} // namespace portable_pwm_fixture
