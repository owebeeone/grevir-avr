#pragma once
#include <grevir/peripherals/pwm/allocator.hpp>
#include <grevir/avr/devices/atmega328p/timers.hpp>

namespace grevir::pwm::atmega328p {
namespace b = ardo::sys::avr::base;
namespace d = ardo::sys::avr::arch_atmega328p;

template <typename PortRegister, unsigned Bit>
consteval unsigned physical_pin() {
  static_assert(Bit < 8, "GREVIR_AVR_INVALID_PIN_BIT");
  return static_cast<unsigned>(PortRegister::addr * 8 + Bit);
}

template <typename Pin>
consteval unsigned physical_pin() {
  return physical_pin<typename Pin::PortReg::register_def, Pin::PortBit::max_bits>();
}

template <typename Bindings>
inline constexpr std::array resources{
  Resource{1,0,Kind::timer}, Resource{2,0,Kind::timer}, Resource{3,0,Kind::timer},
  Resource{11,1,Kind::channel}, Resource{12,1,Kind::channel},
  Resource{21,2,Kind::channel}, Resource{22,2,Kind::channel},
  Resource{31,3,Kind::channel}, Resource{32,3,Kind::channel},
  Resource{physical_pin<typename Bindings::Timer0Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),0,Kind::pin},
  Resource{physical_pin<typename Bindings::Timer0Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),0,Kind::pin},
  Resource{physical_pin<typename Bindings::Timer1Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),0,Kind::pin},
  Resource{physical_pin<typename Bindings::Timer1Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),0,Kind::pin},
  Resource{physical_pin<typename Bindings::Timer2Def::template OcrType<b::OcrEnum::OcrA>::GpioDef>(),0,Kind::pin},
  Resource{physical_pin<typename Bindings::Timer2Def::template OcrType<b::OcrEnum::OcrB>::GpioDef>(),0,Kind::pin}
};

struct Hardware {
  unsigned timer = 0; // Zero-based device timer number.
  unsigned cs = 0;
  unsigned wgm = 0;
  std::uint16_t top = 0;
  b::TimerTop source = b::TimerTop::none;
  constexpr bool operator==(const Hardware&) const = default;
};
struct Choice { Candidate candidate; Hardware hardware; };

namespace nfp {
template <typename T, std::size_t Capacity>
struct BoundedBuffer {
  T entries[Capacity == 0 ? 1 : Capacity]{};
  std::size_t length = 0;
  constexpr void push_back(const T& entry) { entries[length++] = entry; }
  constexpr std::size_t size() const { return length; }
  constexpr bool empty() const { return length == 0; }
  constexpr T* begin() { return entries; }
  constexpr T* end() { return entries + length; }
  constexpr const T* begin() const { return entries; }
  constexpr const T* end() const { return entries + length; }
};
}

constexpr unsigned hash_word(unsigned hash, unsigned value) {
  for (unsigned i = 0; i < 4; ++i) {
    hash = (hash ^ (value & 0xffu)) * 16777619u;
    value >>= 8;
  }
  return hash;
}
constexpr unsigned hash_text(unsigned hash, std::string_view value) {
  for (char c : value) {
    hash = (hash ^ static_cast<unsigned char>(c)) * 16777619u;
  }
  return hash_word(hash, static_cast<unsigned>(value.size()));
}
constexpr unsigned stable_configuration(const Hardware& h) {
  unsigned hash = 2166136261u;
  hash = hash_word(hash, h.timer);
  hash = hash_word(hash, h.cs);
  hash = hash_word(hash, h.wgm);
  hash = hash_word(hash, h.top);
  hash = hash_word(hash, static_cast<unsigned>(h.source));
  return hash == 0 ? 1 : hash;
}
constexpr unsigned stable_choice(const Candidate& c) {
  unsigned hash = hash_word(2166136261u, c.configuration);
  hash = hash_text(hash, c.endpoints[0].request.instance);
  for (unsigned i = 0; i < c.count; ++i) {
    hash = hash_text(hash, c.endpoints[i].request.local);
    hash = hash_word(hash, c.endpoints[i].channel);
    hash = hash_word(hash, c.endpoints[i].pin);
  }
  return hash == 0 ? 1 : hash;
}

template <typename T> struct Types;
template <typename... T> struct Types<b::WaveformGeneratorModes<T...>> {
  static constexpr std::size_t count = sizeof...(T);
  template <typename F> static constexpr void each(F f) { (f.template operator()<T>(), ...); }
};
template <typename... T> struct Types<b::DividerMappings<T...>> {
  static constexpr std::size_t count = sizeof...(T);
  template <typename F> static constexpr void each(F f) { (f.template operator()<T>(), ...); }
};

constexpr Source source(b::TimerTop top) {
  if (top == b::TimerTop::icr) { return Source::icr; }
  if (top == b::TimerTop::ocra) { return Source::ocra; }
  return Source::built_in;
}

// Find the greatest cycle count whose frequency is >= the lower bound.
// Binary search takes at most 16 comparisons, including a zero lower bound.
constexpr std::uint32_t longest_period(FrequencyWindow window, std::uint32_t clock,
    std::uint32_t divider, std::uint32_t maximum) {
  std::uint32_t low = 4, high = maximum, best = 0;
  while (low <= high) {
    const auto middle = low + (high - low) / 2;
    const FrequencyBound actual{std::uint64_t{clock} * 1'000'000, divider * middle};
    if (window.lower.at_most(actual)) { best = middle; low = middle + 1; }
    else { high = middle - 1; }
  }
  return best;
}

template <typename Bindings, unsigned Timer, typename Def, typename Clocks,
    std::uint32_t Clock, typename Requests>
constexpr void append(auto& choices, const Requests& members) {
  FrequencyWindow window;
  for (const auto& r : members) { window = window.intersect(r.config.frequency); }
  if (!window.valid() || window.empty() || members.empty() || members.size() > 2) { return; }
  Types<typename Def::ModeTraits::Modes>::each([&]<typename Mode> {
    if constexpr (Mode::timer_pwm_mode == b::TimerPwmMode::fast) {
      Types<typename Clocks::FreqMapping>::each([&]<typename Divider> {
        constexpr auto capacity = b::nfp::TimerCountField<typename Def::BitsTCNT>::capacity;
        const auto cycles = Mode::timer_top == b::TimerTop::built_in ? Mode::built_in_top + 1
          : longest_period(window, Clock, Divider::divider, capacity + 1);
        if (cycles < 4) { return; }
        const Ratio frequency = Ratio{Clock, Divider::divider * cycles}.normalized();
        if (!window.contains(frequency)) { return; }
        Candidate c;
        c.timer = Timer + 1;
        c.counter_bits = b::nfp::TimerCountField<typename Def::BitsTCNT>::width;
        // Prefer the smallest prescaler, then the numeric WGM code. Within a
        // programmable mode the longest acceptable period maximizes resolution.
        c.preference = Divider::divider * 16 + static_cast<unsigned>(Mode::wgm_value);
        c.frequency = frequency;
        c.waveform = Waveform::fast;
        c.source = source(Mode::timer_top);
        for (const auto& r : members) {
          if (r.config.error != ConfigError::none || !r.config.step.valid()
              || (r.config.required_timer != 0 && r.config.required_timer != c.timer)
              || r.config.counter_bits_at_least > c.counter_bits
              || !at_most({1,cycles}, r.config.step)
              || (r.config.waveform != Waveform::any && r.config.waveform != c.waveform)
              || (r.config.source != Source::any && r.config.source != c.source)) { return; }
          using A = typename Def::template OcrType<b::OcrEnum::OcrA>::GpioDef;
          using B = typename Def::template OcrType<b::OcrEnum::OcrB>::GpioDef;
          unsigned channel = 0;
          if (r.config.pin == physical_pin<A>() && Mode::timer_top != b::TimerTop::ocra) {
            channel = 1;
          } else if (r.config.pin == physical_pin<B>()) { channel = 2; }
          if (channel == 0 || (c.count != 0 && c.endpoints[0].pin == r.config.pin)) { return; }
          c.endpoints[c.count++] = {r.key, (Timer + 1) * 10 + channel, r.config.pin, {1,cycles}};
        }
        Hardware h{Timer, static_cast<unsigned>(Divider::cs_value), static_cast<unsigned>(Mode::wgm_value),
          static_cast<std::uint16_t>(cycles - 1), Mode::timer_top};
        c.configuration = stable_configuration(h);
        c.key = stable_choice(c);
        choices.push_back({c,h});
      });
    }
  });
}

template <typename Bindings, std::uint32_t Clock, std::size_t N>
consteval auto generate(std::array<Request, N> requests) {
  static_assert(Clock > 0 && Clock <= 20'000'000, "ATmega328P clock outside MVP range");
  // Each distinct instance can yield no more than one choice for each
  // timer/mode/divider combination. The bound follows device traits.
  constexpr std::size_t capacity = N * (
    Types<typename Bindings::Timer0Def::ModeTraits::Modes>::count
      * Types<typename d::TccrEnumTraits<d::EnumCS0>::FreqMapping>::count
    + Types<typename Bindings::Timer1Def::ModeTraits::Modes>::count
      * Types<typename d::TccrEnumTraits<d::EnumCS1>::FreqMapping>::count
    + Types<typename Bindings::Timer2Def::ModeTraits::Modes>::count
      * Types<typename d::TccrEnumTraits<d::EnumCS2>::FreqMapping>::count);
  std::sort(requests.begin(), requests.end(), [](const auto& a, const auto& b) { return a.key < b.key; });
  nfp::BoundedBuffer<Choice, capacity> result;
  for (std::size_t i = 0; i < requests.size(); ++i) {
    bool previous = false;
    for (std::size_t j = 0; j < i; ++j) {
      if (requests[j].key.instance == requests[i].key.instance) { previous = true; }
    }
    if (previous) { continue; }
    nfp::BoundedBuffer<Request, N> members;
    members.push_back(requests[i]);
    for (std::size_t j = i + 1; j < requests.size(); ++j) {
      if (requests[j].key.instance == requests[i].key.instance) { members.push_back(requests[j]); }
    }
    append<Bindings,0,typename Bindings::Timer0Def,d::TccrEnumTraits<d::EnumCS0>,Clock>(result,members);
    append<Bindings,1,typename Bindings::Timer1Def,d::TccrEnumTraits<d::EnumCS1>,Clock>(result,members);
    append<Bindings,2,typename Bindings::Timer2Def,d::TccrEnumTraits<d::EnumCS2>,Clock>(result,members);
  }
  return result;
}

} // namespace grevir::pwm::atmega328p
