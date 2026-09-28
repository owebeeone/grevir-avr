#pragma once
#include <grevir/avr/devices/atmega328p/pwm_candidates.hpp>
#include <grevir/core/allocated_application.hpp>
#include <grevir/peripherals/timer/owner_allocator.hpp>
#include <grevir/base/compat/utility.hpp>

namespace grevir::pwm::atmega328p {

template <typename Requests> struct OwnerScope;
template <>
struct OwnerScope<setl::TypeArgs<>> {
  inline static constexpr Text name{""};
  template <Text> static consteval bool declares_use() { return false; }
};
template <typename First, typename... Rest>
struct OwnerScope<setl::TypeArgs<First, Rest...>> {
  static_assert(((First::name.view() == Rest::name.view()) && ...),
    "GREVIR_MODULE_MUST_DECLARE_ONE_TIMER_OWNER");
  inline static constexpr auto name = First::name;
  template <Text Local>
  static consteval bool declares_use() {
    bool found = false;
    First::visit([&]<typename Use>(Use*) {
      if (Use::name.view() == Local.view()) { found = true; }
    });
    return found;
  }
};

template <unsigned... Ids> struct Reservations {
  inline static constexpr std::array<unsigned, sizeof...(Ids)> values{Ids...};
};

struct WidthContradiction {
  unsigned timer = 0;
  unsigned required = 0;
  unsigned available = 0;
};
template <std::size_t N>
struct AllocationPlan {
  Diagnostic diagnostic{};
  std::array<Key, N> requests{};
  std::array<Identity, N> candidates{};
  std::uint32_t visited = 0;
  constexpr bool ok() const { return diagnostic.status == Status::success; }
};
template <auto Owner, Target Resident, unsigned Timer, unsigned Required,
    unsigned Available>
struct WidthGate {
  static_assert(Required <= Available,
    "GREVIR_TIMER_REQUIRED_WIDTH_EXCEEDS_EXPLICIT_TIMER");
  static constexpr bool value = true;
};

// Fixed-frequency PWM binding. All allocation/generation is constant
// evaluated; only fixed register operations and bounded duty arithmetic run.
template <typename Bindings, std::uint32_t Clock, typename Reserved, typename... Instances>
struct Allocation {
private:
  template <typename Requests_, template <typename> typename Module,
    typename Claims_, typename... Dependencies>
  friend struct ::grevir::RequestedModule;
  template <typename Allocation_, typename Requests_>
  friend struct ::grevir::nfp::SelectedTimerParameter;
  template <std::size_t Index> struct Selected;
  template <Text InstanceName, Text LocalName> struct RawPwm;
  template <typename Requests>
  struct ViewStorage {
    struct type {
    template <Text Local>
    struct Binding {
      static_assert(OwnerScope<Requests>::template declares_use<Local>(),
        "GREVIR_FOREIGN_TIMER_USE");
      using type = RawPwm<OwnerScope<Requests>::name,Local>;
    };
    template <Text Local>
    using Pwm = typename Binding<Local>::type;
    };
  };
  template <typename Requests>
  using View = typename ViewStorage<Requests>::type;
public:
  inline static constexpr auto input = requests<Target::atmega328p, Instances...>();
  template <typename Instance>
  inline static constexpr auto width_contradiction = [] {
    constexpr std::array<unsigned, 3> available{
      b::nfp::TimerCountField<typename Bindings::Timer0Def::BitsTCNT>::width,
      b::nfp::TimerCountField<typename Bindings::Timer1Def::BitsTCNT>::width,
      b::nfp::TimerCountField<typename Bindings::Timer2Def::BitsTCNT>::width};
    for (const auto& request : input) {
      if (request.key.instance != Instance::name.view()) { continue; }
      const auto timer = request.config.required_timer;
      if (timer != 0 && timer <= available.size()
          && request.config.counter_bits_at_least > available[timer - 1]) {
        return WidthContradiction{timer,request.config.counter_bits_at_least,
          available[timer - 1]};
      }
    }
    return WidthContradiction{};
  }();
  static_assert((WidthGate<Instances::name,Target::atmega328p,
    width_contradiction<Instances>.timer,width_contradiction<Instances>.required,
    width_contradiction<Instances>.available>::value && ...));
  inline static constexpr auto choices = [] {
    constexpr auto count = generate<Bindings,Clock>(input).size();
    const auto generated = generate<Bindings,Clock>(input);
    std::array<Choice,count> result{};
    std::copy(generated.begin(), generated.end(), result.begin());
    return result;
  }();
  inline static constexpr auto owner_problem = [] {
    std::array<timer::Candidate<Identity>,choices.size()> candidates{};
    for (std::size_t i = 0; i < choices.size(); ++i) {
      const auto& source = choices[i].candidate;
      auto& destination = candidates[i];
      destination.owner = source.endpoints[0].request.instance;
      destination.identity = choices[i].identity();
      destination.timer = source.timer;
      destination.preference = source.preference;
      destination.binding_count = source.count;
      for (unsigned e = 0; e < source.count; ++e) {
        const auto& endpoint = source.endpoints[e];
        destination.bindings[e] = {{endpoint.request.instance,endpoint.request.local},
          timer::UseKind::pwm,endpoint.channel,endpoint.pin};
      }
    }
    return timer::Problem{timer::demands<Instances...>(),candidates,Reserved::values};
  }();
  inline static constexpr auto owner_plan = timer::compile(owner_problem);
  inline static constexpr auto plan = [] {
    AllocationPlan<input.size()> result;
    result.visited = owner_plan.visited;
    for (std::size_t i = 0; i < input.size(); ++i) {
      result.requests[i] = {owner_plan.uses[i].owner,owner_plan.uses[i].local};
      result.candidates[i] = owner_plan.candidates[i];
    }
    const auto error = owner_plan.diagnostic.status;
    result.diagnostic = {
      error == timer::Status::success ? Status::success
        : error == timer::Status::invalid_identity ? Status::invalid_identity
        : error == timer::Status::duplicate_identity ? Status::duplicate_identity
        : error == timer::Status::invalid_model ? Status::model_error
        : error == timer::Status::no_candidate ? Status::no_candidate
        : error == timer::Status::reserved ? Status::reserved
        : error == timer::Status::exhausted ? Status::exhausted : Status::conflict,
      {owner_plan.diagnostic.use.owner,owner_plan.diagnostic.use.local},
      owner_plan.diagnostic.detail};
    return result;
  }();

  template <std::size_t Index>
  inline static constexpr bool used = [] {
    for (const auto& key : plan.candidates) {
      if (key == choices[Index].identity()) { return true; }
    }
    return false;
  }();
  template <std::size_t I, std::size_t E>
  using EndpointClaim = ardo::GPIOResource<choices[I].candidate.endpoints[E].pin>;
  template <std::size_t I, std::size_t... E>
  static auto choice_claims(std::index_sequence<E...>) -> setl::TypeArgs<
    ardo::HardwareTimer<choices[I].hardware.timer>,
    EndpointClaim<I, E>...>;
  template <std::size_t I>
  using ChoiceClaims = std::conditional_t<used<I>, decltype(choice_claims<I>(
    std::make_index_sequence<choices[I].candidate.count>{})),setl::TypeArgs<>>;
  template <std::size_t... I>
  static auto claims(std::index_sequence<I...>) -> typename setl::TypeArgs<>
    ::template cat_type_arg<typename grevir::nfp::Join<ChoiceClaims<I>...>::type>
    ::template eval<ardo::ResourceClaim>;
  using Claims = decltype(claims(std::make_index_sequence<choices.size()>{}));

  template <auto Name>
  inline static constexpr std::size_t owner_index = [] {
    for (std::size_t i = 0; i < plan.requests.size(); ++i) {
      if (plan.requests[i].instance == Name.view()) {
        for (std::size_t c = 0; c < choices.size(); ++c) {
          if (choices[c].identity() == plan.candidates[i]) { return c; }
        }
      }
    }
    return choices.size();
  }();
  template <auto Name>
  using OwnerClaims = typename ChoiceClaims<owner_index<Name>>::template eval<ardo::ResourceClaim>;
private:
  template <auto Name>
  static void setup_owner() {
    static_assert(plan.ok() && owner_index<Name> < choices.size(),
      "GREVIR_TIMER_OWNER_BINDING_UNAVAILABLE");
    Selected<owner_index<Name>>::setup();
  }

private:
  template <std::size_t Index>
  struct Selected {
    inline static constexpr auto hardware = choices[Index].hardware;
    using TimerDef = std::tuple_element_t<hardware.timer,
      std::tuple<typename Bindings::Timer0Def, typename Bindings::Timer1Def, typename Bindings::Timer2Def>>;
    using Registers = typename TimerDef::Registers;
    struct Config {
      static constexpr b::TimerTop timer_top = hardware.source;
      static constexpr std::uint32_t top_count = hardware.top;
      static constexpr std::uint32_t capacity = b::nfp::TimerCountField<typename TimerDef::BitsTCNT>::capacity;
    };
    template <b::OcrEnum Channel>
    using Output = b::TimerOutputPin<b::TimerOutputPinSettings<Channel,true>, Selected>;

    template <b::OcrEnum Channel>
    static bool write(std::uint16_t numerator, std::uint16_t denominator) {
      if (denominator == 0 || numerator > denominator) { return false; }
      constexpr std::uint32_t cycles = std::uint32_t{hardware.top} + 1;
      // <=65536*65535 fits uint32_t, including under the AVR 16-bit int ABI.
      // Round down to a realizable number of high ticks. No FP or 64-bit work.
      const auto ticks = (cycles * std::uint32_t{numerator}) / denominator;
      return writeTicks<Channel>(ticks);
    }
    template <b::OcrEnum Channel>
    static bool writeTicks(std::uint32_t ticks) {
      using Pin = Output<Channel>;
      constexpr std::uint32_t cycles = std::uint32_t{hardware.top} + 1;
      if (ticks > cycles) { return false; }
      if (ticks == 0 || ticks == cycles) {
        Pin::pwmWrite(ticks == 0 ? 0u : hardware.top, hardware.top);
      } else {
        // Fast PWM OCR=0 represents a one-tick pulse, not a zero-duty endpoint.
        Pin::pwmWriteAbsoluteValue(static_cast<typename Pin::OCR::type>(ticks - 1));
        Pin::setupTimerOutputMode();
      }
      return true;
    }

    template <b::OcrEnum Channel>
    static void initialize_output() {
      write<Channel>(0,1);
      Output<Channel>::setupGpio();
    }
    static void setup() {
      using CS = typename TimerDef::BitsCS;
      using WGM = typename TimerDef::BitsWGM_16;
      using Count = typename TimerDef::BitsTCNT;
      using COMA = typename TimerDef::template OcrType<b::OcrEnum::OcrA>::COM8;
      using COMB = typename TimerDef::template OcrType<b::OcrEnum::OcrB>::COM8;
      // Establish stopped normal mode before writing buffered TOP registers.
      Registers::ReadModifyWrite(CS{static_cast<typename CS::type>(0)},
        WGM{static_cast<typename WGM::type>(0)});
      Registers::ReadModifyWrite(COMA{COMA::type::disconnect}, COMB{COMB::type::disconnect});
      Registers::ReadModifyWrite(Count{0});
      if constexpr (hardware.source != b::TimerTop::built_in) {
        using TOP = typename TimerDef::template TimerDefTopRegister<hardware.source>;
        Registers::ReadModifyWrite(TOP{static_cast<typename TOP::type>(hardware.top)});
      }
      for_each_output(std::make_index_sequence<choices[Index].candidate.count>{});
      Registers::ReadModifyWrite(WGM{static_cast<typename WGM::type>(hardware.wgm)});
      Registers::ReadModifyWrite(CS{static_cast<typename CS::type>(hardware.cs)});
    }
    template <std::size_t... E>
    static void for_each_output(std::index_sequence<E...>) {
      (initialize_output<choices[Index].candidate.endpoints[E].channel % 10 == 1
        ? b::OcrEnum::OcrA : b::OcrEnum::OcrB>(), ...);
    }
  };

  template <std::size_t Index>
  static void setup_selected() {
    if constexpr (used<Index>) { Selected<Index>::setup(); }
  }
  template <std::size_t... I>
  static void setup_all(std::index_sequence<I...>) { (setup_selected<I>(), ...); }
  // Caller owns the timers exclusively, enables their peripheral clocks and
  // leaves Timer2 synchronous (AS2=0), with timer interrupts disabled. Startup
  // is sequential; this does not promise glitch-free live reconfiguration.
  static void setup() {
    require_success<plan.diagnostic.status>();
    setup_all(std::make_index_sequence<choices.size()>{});
  }

private:
  template <Text InstanceName, Text LocalName>
  struct RawPwm {
    using Claims = ardo::ResourceClaim<>; // The single application owner holds physical claims.
    static void runSetup() {}
    static void runLoop() {}
    inline static constexpr Key key{InstanceName.view(),LocalName.view()};
    inline static constexpr auto index = [] {
      for (std::size_t i = 0; i < plan.requests.size(); ++i) {
        if (plan.requests[i] == key) {
          for (std::size_t c = 0; c < choices.size(); ++c) {
            if (choices[c].identity() == plan.candidates[i]) { return c; }
          }
        }
      }
      return choices.size();
    }();
    static_assert(plan.ok() && index < choices.size(), "GREVIR_TIMER_BINDING_UNAVAILABLE");
    inline static constexpr auto channel = [] {
      for (const auto& endpoint : choices[index].candidate.endpoints) {
        if (endpoint.request == key) {
          return endpoint.channel % 10 == 1 ? b::OcrEnum::OcrA : b::OcrEnum::OcrB;
        }
      }
      return b::OcrEnum::OcrA;
    }();
    inline static constexpr Ratio actual_frequency = choices[index].candidate.frequency;
    inline static constexpr Ratio duty_step{1,std::uint32_t{choices[index].hardware.top} + 1};
    static bool writeTicks(std::uint32_t high_ticks) {
      return Selected<index>::template writeTicks<channel>(high_ticks);
    }
    static bool write(std::uint16_t numerator, std::uint16_t denominator) {
      return Selected<index>::template write<channel>(numerator,denominator);
    }
  };
};

template <typename Bindings, std::uint32_t Clock, typename... Instances>
using Program = Allocation<Bindings,Clock,Reservations<>,Instances...>;

} // namespace grevir::pwm::atmega328p
