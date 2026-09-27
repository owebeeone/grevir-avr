#include <grevir/avr/devices/atmega328p/pwm_backend.hpp>
#include <grevir/peripherals/timer/own.hpp>
namespace p = grevir::pwm;
namespace a = p::atmega328p;
struct Bytes {
  template <typename T> static T read(std::ptrdiff_t);
  template <typename T> static void write(std::ptrdiff_t,T);
  template <typename T> static void modify(std::ptrdiff_t,T,T);
};
struct Barrier { Barrier(); ~Barrier(); };
using Device = ardo::sys::avr::arch_atmega328p::TimerBindings<Bytes,Barrier>;
inline constexpr unsigned pin_b1 = a::physical_pin<typename Device::Gpio::ppPB1>();
inline constexpr unsigned pin_b2 = a::physical_pin<typename Device::Gpio::ppPB2>();
inline constexpr unsigned pin_d6 = a::physical_pin<typename Device::Gpio::ppPD6>();
using Request = p::Instance<"motor",p::PwmRequest<"pwm",p::Frequency<p::Hertz<1000>,p::Exact>,
  p::DutyStepAtMost<1,256>,p::Pin<pin_b1>,p::avr::TopFromIcr>>;
template <typename Claim>
struct Parameter {
  using Claims = ardo::ResourceClaim<Claim>;
  static void runSetup() {}
  static void runLoop() {}
};
template <typename Plan>
struct Motor : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"pwm">>> {};
using MotorModule = grevir::RequestedModule<setl::TypeArgs<Request>,Motor>;
#if PWM_CASE == 12
template <typename> struct RebindView;
template <template <typename> typename ViewTemplate, typename OldRequests>
struct RebindView<ViewTemplate<OldRequests>> {
  using type = ViewTemplate<setl::TypeArgs<Request>>;
};
#endif
#if PWM_CASE == 1
using Reserved = ardo::HardwareTimer<1>;
#elif PWM_CASE == 2
using Reserved = ardo::GPIOResource<pin_b1>;
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
#elif PWM_CASE == 8
using OtherRequest = p::Instance<"motor",p::PwmRequest<"other",
  p::Frequency<p::Hertz<1000>,p::Exact>,p::DutyStepAtMost<1,256>,
  p::Pin<pin_b2>,p::avr::TopFromIcr>>;
template <typename Plan>
struct OtherMotor : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"other">>> {};
using Extra = grevir::RequestedModule<setl::TypeArgs<OtherRequest>,OtherMotor>;
#elif PWM_CASE == 9
template <typename Plan>
struct Foreign : ardo::ModuleBase<ardo::Parameters<typename Plan::template Pwm<"pwm">>> {};
using Extra = grevir::RequestedModule<setl::TypeArgs<>,Foreign>;
#elif PWM_CASE == 12
template <typename Plan>
struct Foreign : ardo::ModuleBase<ardo::Parameters<>> {
  using Forged = typename RebindView<Plan>::type;
  using Output = typename Forged::template Pwm<"pwm">;
  static void runSetup() { Output::write(1,2); }
};
using Extra = grevir::RequestedModule<setl::TypeArgs<>,Foreign>;
#else
using Extra = grevir::ExistingModule<Existing>;
#endif
#if PWM_CASE == 7
using Wrong = grevir::timer::Instance<"wrong",grevir::timer::Own<
  p::PwmRequest<"pwm",p::Frequency<p::Hertz<15625,16>,p::Exact>,
    p::DutyStepAtMost<1,256>,p::Pin<pin_d6>,p::avr::BuiltInTop>,
  grevir::timer::For<p::Target::avr,grevir::timer::CounterBitsAtLeast<16>>,
  grevir::timer::For<p::Target::atmega328p,
    grevir::timer::RequireTimer<grevir::timer::atmega328p::Timer0>>>>;
template <typename> struct BadMotor : ardo::ModuleBase<ardo::Parameters<>> {};
using BadModule = grevir::RequestedModule<setl::TypeArgs<Wrong>,BadMotor>;
using App = grevir::AllocatedApplication<a::Backend<Device,16000000>,BadModule>;
#else
using App = grevir::AllocatedApplication<a::Backend<Device,16000000>,MotorModule,Extra>;
#endif
#if PWM_CASE == 10
using Forbidden = typename App::Allocation::template RawPwm<"motor","pwm">;
static_assert(sizeof(Forbidden) > 0);
#elif PWM_CASE == 11
using Forged = typename App::Allocation::template View<setl::TypeArgs<Request>>;
static_assert(sizeof(Forged) > 0);
#elif PWM_CASE == 13
void foreign_setup() { App::Allocation::template setup_owner<Request::name>(); }
#elif PWM_CASE == 14
void foreign_setup() { App::Allocation::setup(); }
#endif
static_assert(sizeof(App)>0);
void instantiate() { App::runSetup(); App::runLoop(); }
