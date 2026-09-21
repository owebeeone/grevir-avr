#pragma once
#include <grevir/avr/devices/atmega328p/pwm_program.hpp>

namespace grevir::pwm::atmega328p {
namespace nfp {

// Canonical physical IDs are shared with the capability inventory. A whole
// timer claim also conflicts with any range claim on that HardwareTimer type.
template <unsigned Timer, typename... Claims>
constexpr void reserve_timer(std::vector<unsigned>& ids) {
  if constexpr ((ardo::has_conflict<ardo::HardwareTimer<Timer>,Claims>::value || ...)) {
    if (std::find(ids.begin(),ids.end(),Timer+1) == ids.end()) { ids.push_back(Timer+1); }
  }
}
template <unsigned Pin, typename... Claims>
constexpr void reserve_pin(std::vector<unsigned>& ids) {
  if constexpr ((ardo::has_conflict<ardo::GPIOResource<Pin>,Claims>::value || ...)) {
    if (std::find(ids.begin(),ids.end(),Pin) == ids.end()) { ids.push_back(Pin); }
  }
}
template <typename Explicit, typename... Claims>
struct ReservationsFromClaims {
  static constexpr auto collect() {
    std::vector<unsigned> ids(Explicit::values.begin(),Explicit::values.end());
    reserve_timer<0,Claims...>(ids); reserve_timer<1,Claims...>(ids); reserve_timer<2,Claims...>(ids);
    reserve_pin<PB1,Claims...>(ids); reserve_pin<PB2,Claims...>(ids); reserve_pin<PB3,Claims...>(ids);
    reserve_pin<PD3,Claims...>(ids); reserve_pin<PD5,Claims...>(ids); reserve_pin<PD6,Claims...>(ids);
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
  using type = Allocation<Bindings,Clock,ReservationsFromClaims<Reserved,Claims...>,Requests...>;
};
} // namespace nfp

template <typename Bindings, std::uint32_t Clock, typename Reserved = Reservations<>>
struct Backend {
  template <typename Requests, typename Claims>
  using Allocate = typename nfp::Allocate<Bindings,Clock,Reserved,Requests,Claims>::type;
};

} // namespace grevir::pwm::atmega328p
