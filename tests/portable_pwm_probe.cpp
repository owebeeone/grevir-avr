#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
namespace p = grevir::pwm;
namespace a = p::atmega328p;
struct Bytes {
  template <typename T> static T read(std::ptrdiff_t);
  template <typename T> static void write(std::ptrdiff_t,T);
  template <typename T> static void modify(std::ptrdiff_t,T,T);
};
struct Barrier { Barrier(); ~Barrier(); };
using Device = ardo::sys::avr::arch_atmega328p::TimerBindings<Bytes,Barrier>;
using Request = p::Instance<"motor",p::PwmRequest<"pwm",p::Frequency<p::Hertz<1000>,p::Exact>,
  p::DutyStepAtMost<1,256>,p::Pin<a::PB1>,p::avr::TopFromIcr>>;
template <typename Claim>
struct Parameter {
  using Claims = ardo::ResourceClaim<Claim>;
  static void runSetup() {}
  static void runLoop() {}
};
template <typename Plan>
struct Motor : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"motor">>> {};
using MotorModule = grevir::RequestedModule<setl::TypeArgs<Request>,Motor>;
#if PWM_CASE == 1
using Reserved = ardo::HardwareTimer<1>;
#elif PWM_CASE == 2
using Reserved = ardo::GPIOResource<a::PB1>;
#elif PWM_CASE == 3
using Reserved = ardo::range_claim<ardo::HardwareTimer<1>,0,1>;
#elif PWM_CASE == 6
using Reserved = ardo::shared_use_claim<ardo::HardwareTimer<1>,0,void>;
#else
struct Reserved {};
#endif
struct Existing : ardo::ModuleBase<ardo::Parameters<Parameter<Reserved>>> {};
#if PWM_CASE == 4
// Even undeclared late claims cannot escape the final Core conflict check.
template <typename Plan>
struct LateClaim : ardo::ModuleBase<ardo::Parameters<Parameter<ardo::HardwareTimer<1>>>> {};
using Extra = grevir::RequestedModule<setl::TypeArgs<>,LateClaim>;
#elif PWM_CASE == 5
using Extra = grevir::RequestedModule<setl::TypeArgs<Request>,Motor,ardo::ResourceClaim<Reserved>>;
#else
using Extra = grevir::ExistingModule<Existing>;
#endif
using App = grevir::AllocatedApplication<a::Backend<Device,16000000>,MotorModule,Extra>;
static_assert(sizeof(App)>0);
void instantiate() { App::runSetup(); App::runLoop(); }
