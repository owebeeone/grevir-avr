#pragma once
#include <grevir/avr/devices/atmega328p/pwm_program.hpp>

namespace grevir::pwm::atmega328p {
namespace nfp {

// Canonical physical IDs are shared with the capability inventory. A whole
// timer claim also conflicts with any range claim on that HardwareTimer type.
template <unsigned Timer, typename... Claims, typename Ids>
constexpr void reserve_timer(Ids& ids) {
  if constexpr ((ardo::has_conflict<ardo::HardwareTimer<Timer>,Claims>::value || ...)) {
    if (std::find(ids.begin(),ids.end(),Timer+1) == ids.end()) { ids.push_back(Timer+1); }
  }
}
template <unsigned Pin, typename... Claims, typename Ids>
constexpr void reserve_pin(Ids& ids) {
  if constexpr ((ardo::has_conflict<ardo::GPIOResource<Pin>,Claims>::value || ...)) {
    if (std::find(ids.begin(),ids.end(),Pin) == ids.end()) { ids.push_back(Pin); }
  }
}
template <typename Bindings, typename Explicit, typename... Claims>
struct ReservationsFromClaims {
  static consteval auto collect() {
    BoundedBuffer<unsigned, Explicit::values.size() + 9> ids;
    for (unsigned id : Explicit::values) { ids.push_back(id); }
    reserve_timer<0,Claims...>(ids); reserve_timer<1,Claims...>(ids); reserve_timer<2,Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer0Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer0Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer1Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer1Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer2Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),Claims...>(ids);
    reserve_pin<physical_pin<typename Bindings::Timer2Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),Claims...>(ids);
    return ids;
  }
  inline static constexpr auto values = [] {
    constexpr auto count = collect().size();
    const auto ids = collect();
    std::array<unsigned,count> result{};
    std::copy(ids.begin(),ids.end(),result.begin());
    return result;
  }();
};

template <typename Bindings, std::uint32_t Clock, typename Reserved, typename Requests, typename Claims>
struct Allocate;
template <typename Bindings, std::uint32_t Clock, typename Reserved, typename... Requests, typename... Claims>
struct Allocate<Bindings,Clock,Reserved,setl::TypeArgs<Requests...>,setl::TypeArgs<Claims...>> {
  using type = Allocation<Bindings,Clock,ReservationsFromClaims<Bindings,Reserved,Claims...>,Requests...>;
};
} // namespace nfp

template <typename Bindings, std::uint32_t Clock, typename Reserved = Reservations<>>
struct Backend {
  template <typename Requests, typename Claims>
  using Allocate = typename nfp::Allocate<Bindings,Clock,Reserved,Requests,Claims>::type;
};

} // namespace grevir::pwm::atmega328p
