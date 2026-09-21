#pragma once

#include <grevir/avr/timer/mode.hpp>
#include <grevir/base/meta/type_algorithms.hpp>
#include <grevir/registers/apply.hpp>
#include <cstdint>
#include <tuple>

namespace ardo::sys::avr::base {

template <typename WgmEnum> struct WgmEnumTraits;

template <typename B>
struct TimerInputNoiseCanceller {
  using Activate = setl::ApplierValues<setl::ApplierValue<B, true>>;
  using Deactivate = setl::ApplierValues<setl::ApplierValue<B, false>>;
  static constexpr bool has_noise_canceller = true;
};

template <>
struct TimerInputNoiseCanceller<void> {
  using Activate = setl::ApplierValues<>;
  using Deactivate = setl::ApplierValues<>;
  static constexpr bool has_noise_canceller = false;
};

/**
 * Configuration information for timer capture. The ICR register is used to
 * capture the timer value when an input capture event occurs or sets the top
 * value for the timer. The noise canceler is used to filter out noise on the
 * input capture pin. The edge selection is used to select the edge of the input
 * capture pin that will trigger the capture event.
 *
 * The ICR register is only available on some timers.
 */
template <typename w_BitsICR, typename w_BitsICNC, typename w_BitsEdge>
struct TimerCapture {
  static constexpr bool has_capture_register = true;
  using CaptureBits = w_BitsICR;
  using NoiseCanceler = w_BitsICNC;
  using Rising = setl::ApplierValues<setl::ApplierValue<w_BitsEdge, true>>;
  using Falling = setl::ApplierValues<setl::ApplierValue<w_BitsEdge, false>>;
  using NoiseCanceller = TimerInputNoiseCanceller<NoiseCanceler>;
  using TopCountProvided = setl::ValueTuple<TimerTop, TimerTop::icr>;
  static constexpr bool has_input_capture = true;
  static constexpr bool has_edge_selection = true;
};

template <>
struct TimerCapture<void, void, void> {
  static constexpr bool has_capture_register = false;
  using CaptureBits = void;
  using Rising = setl::ApplierValues<>;
  using Falling = setl::ApplierValues<>;
  using NoiseCanceller = TimerInputNoiseCanceller<void>;
  using TopCountProvided = setl::ValueTuple<TimerTop>;  // Empty tuple.
  static constexpr bool has_input_capture = false;
  static constexpr bool has_edge_selection = false;
};

enum class OcrEnum : std::uint8_t {
  OcrA = 0,
  OcrB = 1
};

template <typename w_OCR,
          typename w_OCIE,
          typename w_OCF,
          typename w_COM,
          typename w_COM8,
          typename w_GpioDef>
struct OutputCompare {
  using OCR = w_OCR;
  using OCIE = w_OCIE;
  using OCF = w_OCF;
  using COM = w_COM;
  using COM8 = w_COM8;
  using GpioDef = w_GpioDef;
};

/**
 * Specializations for providing the configured timer top value dynamically.
 */

template <typename w_TimerDef, TimerTop timertop>
struct TopGetter;

/** Specialization for fetching from the ICR register. */
template <typename w_TimerDef>
struct TopGetter<w_TimerDef, TimerTop::icr> {
  using TypeWgmEnum = typename w_TimerDef::TypeWgmEnum;
  using Registers = typename w_TimerDef::Registers;
  using TimerCaptureType = typename w_TimerDef::TimerCaptureType;
  using BitsICR = typename TimerCaptureType::CaptureBits;
  using ICRType = typename BitsICR::type;

  static ICRType get() {
    return Registers::template Read<BitsICR>();
  }

  static ICRType get(TypeWgmEnum) {
    return Registers::template Read<BitsICR>();
  }
};

/** Specialization for fetching from the OCRA register. */
template <typename w_TimerDef>
struct TopGetter<w_TimerDef, TimerTop::ocra> {
  using TypeWgmEnum = typename w_TimerDef::TypeWgmEnum;
  using Registers = typename w_TimerDef::Registers;
  using OCR = typename w_TimerDef::template OcrType<OcrEnum::OcrA>::OCR;
  using OCRAType = typename OCR::type;

  static OCRAType get() {
    return Registers::template Read<OCR>();
  }

  static OCRAType get(TypeWgmEnum) {
    return Registers::template Read<OCR>();
  }
};

/** Checked built-in TOP lookup; absent or register-based modes have no result. */
template <typename w_TimerDef>
struct TopGetter<w_TimerDef, TimerTop::built_in> {
  using TypeWgmEnum = typename w_TimerDef::TypeWgmEnum;
  using Registers = typename w_TimerDef::Registers;
  using BitsWGM_16 = typename w_TimerDef::BitsWGM_16;
  using BitsWGM_16_type = typename BitsWGM_16::type;
  using Modes = typename w_TimerDef::ModeTraits::Modes;

  static setl::Optional<std::uint32_t> get() {
    return get(Registers::template Read<BitsWGM_16>());
  }

  static setl::Optional<std::uint32_t> get(TypeWgmEnum wgm) {
    const auto source = Modes::template getParamFor<WaveformGeneratorModeParam::timer_top>(wgm);
    if (!source.is_present() || source.get() != TimerTop::built_in) {
      return {};
    }
    return Modes::template getParamFor<WaveformGeneratorModeParam::built_in_top>(wgm);
  }
};

/**
 * Provides the register used to set the top value for the timer.
 */
template <typename w_TimerDef, TimerTop w_timer_top>
struct TopRegisterFinder;

template <typename w_TimerDef>
struct TopRegisterFinder<w_TimerDef, TimerTop::icr> {
  using TimerCaptureType = typename w_TimerDef::TimerCaptureType;
  using BitsICR = typename TimerCaptureType::CaptureBits;
  using type = BitsICR;
};

template <typename w_TimerDef>
struct TopRegisterFinder<w_TimerDef, TimerTop::ocra> {
  using OutputCompareA = typename w_TimerDef::template OcrType<OcrEnum::OcrA>;
  using type = typename OutputCompareA::OCR;
};

namespace nfp {
template <typename Definition, typename Sources>
struct TimerTopReader;

template <typename Definition>
struct TimerTopReader<Definition, setl::ValueTuple<TimerTop>> {
  template <typename... P>
  static setl::Optional<std::uint32_t> get(TimerTop, const P&...) {
    return {};
  }
};

// Fix the public result representation rather than inferring it from whichever
// native-width register getters happen to be present. Optional results propagate.
template <typename Definition, TimerTop First, TimerTop... Rest>
struct TimerTopReader<Definition, setl::ValueTuple<TimerTop, First, Rest...>>
  : setl::ValueTupleGetterTyped<TimerTop, setl::ValueTuple<TimerTop, First, Rest...>,
      Definition::template TimerDefTopGetter, setl::Optional<std::uint32_t>> {};
} // namespace nfp


/**
 * Defines all the resources for a single AVR timer. This can be used for both
 * 8-bit and 16-bit timers.
 * @Params:
 *  w_BitsWGM_16: The WGM bits in the TCCRnA and TCCRnB registers.
 *  w_BitsCS: The CS bits in the TCCRnB register.
 *  w_BitsFOCA: The FOCnA bit in the TCCRnA register.
 *  w_BitsFOCB: The FOCnB bit in the TCCRnA register.
 *  w_BitsTCNT: The TCNTn register.
 *  w_OutputCompareTuple: A tuple of OutputCompare type containing properties for OCRA functions.
 *  w_TimerCapture: A TimerCapture type containing properties for ICR functions.
 *  w_RegistersTuple: Tuple of registers with caller-supplied access policies.
 *  w_TimerTopTuple: Base TOP sources; capture-provided sources are appended.
 *  w_ModeTraits: Supplies Modes, defaulting to the legacy enum-trait specialization.
 */
template <
  typename w_BitsWGM_16,
  typename w_BitsCS,
  typename w_BitsFOCA,
  typename w_BitsFOCB,
  typename w_BitsTCNT,
  typename w_OutputCompareTuple,
  typename w_TimerCapture,
  typename w_RegistersTuple,
  typename w_TimerTopTuple = setl::ValueTuple<TimerTop, TimerTop::built_in, TimerTop::ocra>,
  typename w_ModeTraits = WgmEnumTraits<typename w_BitsWGM_16::type>
>
struct TimerDefinition
{
  using BitsWGM_16 = w_BitsWGM_16;
  using BitsCS = w_BitsCS;
  using BitsFOCA = w_BitsFOCA;
  using BitsFOCB = w_BitsFOCB;
  using BitsTCNT = w_BitsTCNT;
  using OutputCompareTuple = w_OutputCompareTuple;
  using TimerCaptureType = w_TimerCapture;
  using Registers = setl::RegisterSelector<w_RegistersTuple>;
  using TimerTopTuple = setl::ValueTupleCat<
      w_TimerTopTuple, typename TimerCaptureType::TopCountProvided>;
  using TypeWgmEnum = typename BitsWGM_16::type;
  using ModeTraits = w_ModeTraits;

  template <TimerTop w_timer_top>
  using TimerDefTopGetter = TopGetter<TimerDefinition, w_timer_top>;

  template <TimerTop w_timer_top>
  using TimerDefTopRegister = typename TopRegisterFinder<TimerDefinition, w_timer_top>::type;

  /**
   * The OutputCompare type for the given w_ocr value.
   */
  template <OcrEnum w_ocr>
  using OcrType = std::tuple_element_t<static_cast<std::size_t>(w_ocr), OutputCompareTuple>;


  /** Returns the timer top value for the given timer top enum. */
  static setl::Optional<std::uint32_t> get_timer_top(TimerTop timer_top) {
    return nfp::TimerTopReader<TimerDefinition, TimerTopTuple>::get(timer_top);
  }

  /** Returns the timer top value for the given timer top enum. */
  static setl::Optional<std::uint32_t> get_timer_top(TimerTop timer_top, TypeWgmEnum wgmEnum) {
    return nfp::TimerTopReader<TimerDefinition, TimerTopTuple>::get(timer_top, wgmEnum);
  }
};

} // namespace ardo::sys::avr::base
